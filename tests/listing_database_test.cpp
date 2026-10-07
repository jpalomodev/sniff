#include "adapters/persistence/listing_database.hpp"

#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <stdexcept>

namespace {

class TemporaryDatabase {
  public:
    TemporaryDatabase() {
        const auto base = std::filesystem::temp_directory_path();
        std::random_device random;
        for (int attempt = 0; attempt < 100; ++attempt) {
            directory = base / ("sniff-db-test-" + std::to_string(random()));
            if (std::filesystem::create_directory(directory)) {
                path = directory / "nested" / "listings.db";
                return;
            }
        }
        throw std::runtime_error("Unable to create test directory");
    }
    ~TemporaryDatabase() {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }
    std::filesystem::path directory;
    std::filesystem::path path;
};

sniff::ListingData data() {
    sniff::ListingData result;
    result.source = "fixture";
    result.source_id = "00123";
    result.reference = "fixture:00123";
    return result;
}

const sniff::Listing::CaptureTime captured_at{std::chrono::milliseconds{1700000000123}};

void raw_sql(const std::filesystem::path& path, const char* sql, int expected = SQLITE_OK) {
    sqlite3* raw = nullptr;
    const int result = sqlite3_open(path.string().c_str(), &raw);
    const std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db(raw, sqlite3_close);
    REQUIRE(result == SQLITE_OK);
    REQUIRE(sqlite3_exec(db.get(), sql, nullptr, nullptr, nullptr) == expected);
}

} // namespace

TEST_CASE("Listings survive connection close and reopen", "[persistence]") {
    TemporaryDatabase file;
    auto input = data();
    input.title = "  Seller's instrument  ";
    input.description = std::string{"text\0more", 9};
    input.asking_price = sniff::AskingPrice{12345, "USD"};
    input.location = "Chicago";
    input.shipping = {.available = false, .seller_terms = "Pickup only"};
    input.seller_condition = "Untested";
    input.image_references = {"local:second.jpg", "https://example.test/first.jpg",
                              "local:second.jpg"};
    const sniff::Listing original{input, captured_at};
    std::int64_t id;
    {
        sniff::persistence::ListingDatabase db{file.path};
        id = db.save(original);
    }
    sniff::persistence::ListingDatabase reopened{file.path};
    const auto loaded = reopened.load(id);
    REQUIRE(loaded.has_value());
    CHECK(loaded->source() == original.source());
    CHECK(loaded->source_id() == original.source_id());
    CHECK(loaded->reference() == original.reference());
    CHECK(loaded->title() == original.title());
    CHECK(loaded->description() == original.description());
    CHECK(loaded->asking_price() == original.asking_price());
    CHECK(loaded->location() == original.location());
    CHECK(loaded->shipping() == original.shipping());
    CHECK(loaded->seller_condition() == original.seller_condition());
    CHECK(loaded->image_references() == original.image_references());
    CHECK(loaded->captured_at() == original.captured_at());
}

TEST_CASE("Persistence retains unknown values and explicit zero or empty values", "[persistence]") {
    TemporaryDatabase file;
    sniff::persistence::ListingDatabase db{file.path};
    auto id = db.save(sniff::Listing{data(), captured_at});
    const auto unknown = db.load(id);
    REQUIRE(unknown);
    CHECK_FALSE(unknown->title());
    CHECK_FALSE(unknown->description());
    CHECK_FALSE(unknown->asking_price());
    CHECK_FALSE(unknown->location());
    CHECK_FALSE(unknown->shipping().available);
    CHECK_FALSE(unknown->shipping().seller_terms);
    CHECK_FALSE(unknown->seller_condition());
    CHECK(unknown->image_references().empty());

    auto input = data();
    input.title = "";
    input.asking_price = sniff::AskingPrice{0, "USD"};
    input.shipping.available = true;
    id = db.save(sniff::Listing{input, captured_at + std::chrono::milliseconds{1}});
    const auto known = db.load(id);
    REQUIRE(known);
    REQUIRE(known->title());
    CHECK(known->title()->empty());
    REQUIRE(known->asking_price());
    CHECK(known->asking_price()->minor_units == 0);
    CHECK(known->shipping().available == true);
    CHECK_FALSE(db.load(-1));
}

TEST_CASE("Snapshots retain history and reject duplicate captures", "[persistence]") {
    TemporaryDatabase file;
    sniff::persistence::ListingDatabase db{file.path};
    const sniff::Listing first{data(), captured_at};
    const auto first_id = db.save(first);
    CHECK_THROWS_AS(db.save(first), std::runtime_error);
    const auto second_id =
        db.save(sniff::Listing{data(), captured_at + std::chrono::milliseconds{1}});
    CHECK(first_id != second_id);
    CHECK(db.load(first_id).has_value());
    CHECK(db.load(second_id).has_value());
}

TEST_CASE("Failed image inserts roll back the entire snapshot", "[persistence][recovery]") {
    TemporaryDatabase file;
    sniff::persistence::ListingDatabase db{file.path};
    raw_sql(file.path, "CREATE TRIGGER reject_image BEFORE INSERT ON listing_images "
                       "WHEN NEW.position = 1 BEGIN SELECT RAISE(ABORT, 'test failure'); END");
    auto input = data();
    input.image_references = {"local:one", "local:two"};
    CHECK_THROWS_AS(db.save(sniff::Listing{input, captured_at}), std::runtime_error);
    CHECK_FALSE(db.load(1));
    raw_sql(file.path, "DROP TRIGGER reject_image");
    const auto id = db.save(sniff::Listing{input, captured_at});
    REQUIRE(db.load(id));
    CHECK(db.load(id)->image_references() == input.image_references);
}

TEST_CASE("Schema rejects mismatched price and invalid shipping values",
          "[persistence][validation]") {
    TemporaryDatabase file;
    sniff::persistence::ListingDatabase db{file.path};
    const auto id = db.save(sniff::Listing{data(), captured_at});
    raw_sql(file.path, "UPDATE listings SET asking_price_minor_units = 100", SQLITE_CONSTRAINT);
    raw_sql(file.path, "UPDATE listings SET shipping_available = 2", SQLITE_CONSTRAINT);
    raw_sql(file.path, "UPDATE listings SET asking_price_minor_units = -1, currency = 'USD'",
            SQLITE_CONSTRAINT);
    raw_sql(file.path, "UPDATE listings SET asking_price_minor_units = 1, currency = 'usd'",
            SQLITE_CONSTRAINT);
    raw_sql(file.path, "UPDATE listings SET source_id = ' ' ", SQLITE_CONSTRAINT);
    REQUIRE(db.load(id));
    CHECK_FALSE(db.load(id)->asking_price());
}

TEST_CASE("Database refuses transient paths and incompatible existing files",
          "[persistence][validation]") {
    CHECK_THROWS_AS(sniff::persistence::ListingDatabase{""}, std::invalid_argument);
    CHECK_THROWS_AS(sniff::persistence::ListingDatabase{":memory:"}, std::invalid_argument);
    TemporaryDatabase file;
    {
        sniff::persistence::ListingDatabase db{file.path};
    }
    raw_sql(file.path, "PRAGMA user_version = 99");
    CHECK_THROWS_AS(sniff::persistence::ListingDatabase{file.path}, std::runtime_error);
    raw_sql(file.path, "PRAGMA user_version = 0");
    CHECK_THROWS_AS(sniff::persistence::ListingDatabase{file.path}, std::runtime_error);
    CHECK_THROWS(sniff::persistence::ListingDatabase{file.directory});
    const auto corrupt = file.directory / "corrupt.db";
    std::ofstream(corrupt) << "This is not SQLite";
    CHECK_THROWS_AS(sniff::persistence::ListingDatabase{corrupt}, std::runtime_error);
}

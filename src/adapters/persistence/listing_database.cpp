#include "adapters/persistence/listing_database.hpp"
#include "schema.hpp"

#include <sqlite3.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace sniff::persistence {
namespace {

void check(sqlite3* db, int result) {
    if (result != SQLITE_OK) {
        throw std::runtime_error(std::string("SQLite operation failed: ") + sqlite3_errmsg(db));
    }
}

void execute(sqlite3* db, const char* sql) {
    check(db, sqlite3_exec(db, sql, nullptr, nullptr, nullptr));
}

using Statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

Statement prepare(sqlite3* db, const char* sql) {
    sqlite3_stmt* raw = nullptr;
    const int result = sqlite3_prepare_v2(db, sql, -1, &raw, nullptr);
    Statement statement(raw, sqlite3_finalize);
    check(db, result);
    return statement;
}

int step(sqlite3* db, sqlite3_stmt* statement) {
    const int result = sqlite3_step(statement);
    if (result != SQLITE_ROW && result != SQLITE_DONE) {
        check(db, result);
    }
    return result;
}

void bind(sqlite3* db, sqlite3_stmt* statement, int index, std::string_view value) {
    if (value.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("Listing text exceeds SQLite binding size");
    }
    check(db, sqlite3_bind_text(statement, index, value.data(), static_cast<int>(value.size()),
                                SQLITE_TRANSIENT));
}

void bind_optional(sqlite3* db, sqlite3_stmt* statement, int index,
                   const std::optional<std::string>& value) {
    if (value) {
        bind(db, statement, index, *value);
    } else {
        check(db, sqlite3_bind_null(statement, index));
    }
}

std::string text(sqlite3_stmt* statement, int column) {
    const auto* value = reinterpret_cast<const char*>(sqlite3_column_text(statement, column));
    if (!value) {
        throw std::runtime_error("Missing required SQLite text value");
    }
    return {value, static_cast<std::size_t>(sqlite3_column_bytes(statement, column))};
}

std::optional<std::string> optional_text(sqlite3_stmt* statement, int column) {
    if (sqlite3_column_type(statement, column) == SQLITE_NULL) {
        return std::nullopt;
    }
    return text(statement, column);
}

class Transaction {
  public:
    explicit Transaction(sqlite3* db, const char* begin = "BEGIN IMMEDIATE") : db_(db) {
        execute(db_, begin);
    }
    ~Transaction() {
        if (!committed_) {
            sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        }
    }
    void commit() {
        execute(db_, "COMMIT");
        committed_ = true;
    }

  private:
    sqlite3* db_;
    bool committed_ = false;
};

} // namespace

struct ListingDatabase::Impl {
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> database{nullptr, sqlite3_close};
};

ListingDatabase::ListingDatabase(const std::filesystem::path& path)
    : impl_(std::make_unique<Impl>()) {
    const auto filename = path.string();
    if (filename.empty() || filename == ":memory:" || filename.find('\0') != std::string::npos) {
        throw std::invalid_argument("A persistent database file path is required");
    }
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    sqlite3* raw = nullptr;
    const int result = sqlite3_open_v2(filename.c_str(), &raw,
                                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    impl_->database.reset(raw);
    if (!raw) {
        throw std::runtime_error("Unable to allocate SQLite connection");
    }
    check(raw, result);
    check(raw, sqlite3_busy_timeout(raw, 5000));
    execute(raw, "PRAGMA foreign_keys = ON; PRAGMA synchronous = FULL;");

    Transaction transaction(raw);
    auto version = prepare(raw, "PRAGMA user_version");
    if (step(raw, version.get()) != SQLITE_ROW) {
        throw std::runtime_error("Unable to read database schema version");
    }
    const int schema_version = sqlite3_column_int(version.get(), 0);
    version.reset();
    if (schema_version == 0) {
        auto tables =
            prepare(raw, "SELECT count(*) FROM sqlite_schema WHERE name NOT LIKE 'sqlite_%'");
        if (step(raw, tables.get()) != SQLITE_ROW || sqlite3_column_int64(tables.get(), 0) != 0) {
            throw std::runtime_error("Refusing to initialize a nonempty unversioned database");
        }
        tables.reset();
        execute(raw, schema_sql);
    } else if (schema_version != 1) {
        throw std::runtime_error("Unsupported database schema version");
    }
    transaction.commit();
}

ListingDatabase::~ListingDatabase() = default;

std::int64_t ListingDatabase::save(const Listing& listing) {
    auto* db = impl_->database.get();
    Transaction transaction(db);
    auto insert = prepare(db, R"sql(
        INSERT INTO listings (source, source_id, reference, title, description,
            asking_price_minor_units, currency, location, shipping_available,
            shipping_terms, seller_condition, captured_at_ms)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )sql");
    bind(db, insert.get(), 1, listing.source());
    bind(db, insert.get(), 2, listing.source_id());
    bind(db, insert.get(), 3, listing.reference());
    bind_optional(db, insert.get(), 4, listing.title());
    bind_optional(db, insert.get(), 5, listing.description());
    if (listing.asking_price()) {
        check(db, sqlite3_bind_int64(insert.get(), 6, listing.asking_price()->minor_units));
        bind(db, insert.get(), 7, listing.asking_price()->currency);
    }
    bind_optional(db, insert.get(), 8, listing.location());
    if (listing.shipping().available) {
        check(db, sqlite3_bind_int(insert.get(), 9, *listing.shipping().available ? 1 : 0));
    }
    bind_optional(db, insert.get(), 10, listing.shipping().seller_terms);
    bind_optional(db, insert.get(), 11, listing.seller_condition());
    check(db,
          sqlite3_bind_int64(insert.get(), 12, listing.captured_at().time_since_epoch().count()));
    step(db, insert.get());
    const auto snapshot_id = sqlite3_last_insert_rowid(db);
    auto image = prepare(
        db, "INSERT INTO listing_images (listing_id, position, reference) VALUES (?, ?, ?)");
    std::int64_t position = 0;
    for (const auto& reference : listing.image_references()) {
        check(db, sqlite3_bind_int64(image.get(), 1, snapshot_id));
        check(db, sqlite3_bind_int64(image.get(), 2, position++));
        bind(db, image.get(), 3, reference);
        step(db, image.get());
        check(db, sqlite3_reset(image.get()));
    }
    transaction.commit();
    return snapshot_id;
}

std::optional<Listing> ListingDatabase::load(std::int64_t snapshot_id) const {
    auto* db = impl_->database.get();
    Transaction transaction(db, "BEGIN");
    auto row = prepare(db, R"sql(
        SELECT source, source_id, reference, title, description, asking_price_minor_units,
            currency, location, shipping_available, shipping_terms, seller_condition, captured_at_ms
        FROM listings WHERE id = ?
    )sql");
    check(db, sqlite3_bind_int64(row.get(), 1, snapshot_id));
    if (step(db, row.get()) == SQLITE_DONE) {
        transaction.commit();
        return std::nullopt;
    }
    ListingData data;
    data.source = text(row.get(), 0);
    data.source_id = text(row.get(), 1);
    data.reference = text(row.get(), 2);
    data.title = optional_text(row.get(), 3);
    data.description = optional_text(row.get(), 4);
    if (sqlite3_column_type(row.get(), 5) != SQLITE_NULL) {
        data.asking_price = AskingPrice{sqlite3_column_int64(row.get(), 5), text(row.get(), 6)};
    }
    data.location = optional_text(row.get(), 7);
    if (sqlite3_column_type(row.get(), 8) != SQLITE_NULL) {
        data.shipping.available = sqlite3_column_int(row.get(), 8) != 0;
    }
    data.shipping.seller_terms = optional_text(row.get(), 9);
    data.seller_condition = optional_text(row.get(), 10);
    const Listing::CaptureTime captured_at{
        std::chrono::milliseconds{sqlite3_column_int64(row.get(), 11)}};
    row.reset();
    auto images =
        prepare(db, "SELECT reference FROM listing_images WHERE listing_id = ? ORDER BY position");
    check(db, sqlite3_bind_int64(images.get(), 1, snapshot_id));
    while (step(db, images.get()) == SQLITE_ROW) {
        data.image_references.push_back(text(images.get(), 0));
    }
    Listing listing{std::move(data), captured_at};
    transaction.commit();
    return listing;
}

} // namespace sniff::persistence

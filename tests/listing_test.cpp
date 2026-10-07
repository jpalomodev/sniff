#include "core/listing.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

sniff::ListingData minimal_data() {
    sniff::ListingData data;
    data.source = "fixture";
    data.source_id = "00123";
    data.reference = "fixture:listings/00123";
    return data;
}

const sniff::Listing::CaptureTime capture_time{std::chrono::milliseconds{1700000000123}};

} // namespace

TEST_CASE("Listing preserves captured seller evidence", "[listing]") {
    auto data = minimal_data();
    data.title = "  Unknown instrument  ";
    data.description = "Seller says: 'not tested'\nIncludes cables.";
    data.asking_price = sniff::AskingPrice{12345, "USD"};
    data.location = "Chicago, IL";
    data.shipping = {.available = true, .seller_terms = "Buyer pays shipping"};
    data.image_references = {"https://example.test/image.jpg", "local:photos/back.jpg"};
    data.seller_condition = "Untested / as-is";

    const sniff::Listing listing{data, capture_time};
    CHECK(listing.source() == data.source);
    CHECK(listing.source_id() == "00123");
    CHECK(listing.reference() == data.reference);
    CHECK(listing.title() == data.title);
    CHECK(listing.description() == data.description);
    CHECK(listing.asking_price() == data.asking_price);
    CHECK(listing.location() == data.location);
    CHECK(listing.shipping() == data.shipping);
    CHECK(listing.image_references() == data.image_references);
    CHECK(listing.seller_condition() == data.seller_condition);
    CHECK(listing.captured_at() == capture_time);

    data.title = "Changed input";
    data.image_references.clear();
    CHECK(listing.title() == "  Unknown instrument  ");
    CHECK(listing.image_references().size() == 2);
}

TEST_CASE("Incomplete listings retain unknown values", "[listing][missing-data]") {
    const sniff::Listing listing{minimal_data(), capture_time};
    CHECK_FALSE(listing.title().has_value());
    CHECK_FALSE(listing.description().has_value());
    CHECK_FALSE(listing.asking_price().has_value());
    CHECK_FALSE(listing.location().has_value());
    CHECK_FALSE(listing.shipping().available.has_value());
    CHECK_FALSE(listing.shipping().seller_terms.has_value());
    CHECK_FALSE(listing.seller_condition().has_value());
    CHECK(listing.image_references().empty());
}

TEST_CASE("Zero asking price differs from missing price", "[listing][money]") {
    auto data = minimal_data();
    data.asking_price = sniff::AskingPrice{0, "USD"};
    const sniff::Listing listing{std::move(data), capture_time};
    REQUIRE(listing.asking_price().has_value());
    CHECK(listing.asking_price()->minor_units == 0);
}

TEST_CASE("Unavailable shipping differs from unknown shipping", "[listing][missing-data]") {
    auto data = minimal_data();
    data.shipping.available = false;
    const sniff::Listing listing{std::move(data), capture_time};
    REQUIRE(listing.shipping().available.has_value());
    CHECK_FALSE(*listing.shipping().available);
}

TEST_CASE("Listing rejects missing traceability fields", "[listing][validation]") {
    for (const auto member : {&sniff::ListingData::source, &sniff::ListingData::source_id,
                              &sniff::ListingData::reference}) {
        for (const auto* value : {"", " \t\r\n"}) {
            auto data = minimal_data();
            data.*member = value;
            CHECK_THROWS_AS((sniff::Listing{std::move(data), capture_time}), std::invalid_argument);
        }
    }
}

TEST_CASE("Listing rejects malformed asking prices", "[listing][money][validation]") {
    for (const auto& price : {sniff::AskingPrice{-1, "USD"}, sniff::AskingPrice{100, ""},
                              sniff::AskingPrice{100, "usd"}, sniff::AskingPrice{100, "US"},
                              sniff::AskingPrice{100, "USDD"}, sniff::AskingPrice{100, "U1D"}}) {
        auto data = minimal_data();
        data.asking_price = price;
        CHECK_THROWS_AS((sniff::Listing{std::move(data), capture_time}), std::invalid_argument);
    }
}

TEST_CASE("Listing preserves large prices without floating-point conversion", "[listing][money]") {
    auto data = minimal_data();
    data.asking_price = sniff::AskingPrice{std::numeric_limits<std::int64_t>::max(), "JPY"};
    const sniff::Listing listing{std::move(data), capture_time};
    REQUIRE(listing.asking_price().has_value());
    CHECK(listing.asking_price()->minor_units == std::numeric_limits<std::int64_t>::max());
    CHECK(listing.asking_price()->currency == "JPY");
}

TEST_CASE("Listing rejects blank image references", "[listing][validation]") {
    for (const auto* value : {"", " \t\n"}) {
        auto data = minimal_data();
        data.image_references = {"local:valid.jpg", value};
        CHECK_THROWS_AS((sniff::Listing{std::move(data), capture_time}), std::invalid_argument);
    }
}

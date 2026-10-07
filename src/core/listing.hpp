#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sniff {

// The adapter converts a source price into the currency's minor units.
struct AskingPrice {
    std::int64_t minor_units = 0;
    std::string currency;

    bool operator==(const AskingPrice&) const = default;
};

struct ShippingDetails {
    // Unknown, explicitly unavailable, and available are distinct states.
    std::optional<bool> available;
    std::optional<std::string> seller_terms;

    bool operator==(const ShippingDetails&) const = default;
};

// Input assembled by an adapter; Listing validates it at construction.
struct ListingData {
    std::string source;
    std::string source_id;
    std::string reference;
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<AskingPrice> asking_price;
    std::optional<std::string> location;
    ShippingDetails shipping;
    std::vector<std::string> image_references;
    std::optional<std::string> seller_condition;
};

class Listing {
  public:
    using CaptureTime = std::chrono::sys_time<std::chrono::milliseconds>;

    // Throws std::invalid_argument for malformed identity, price, or image references.
    explicit Listing(ListingData data, CaptureTime captured_at);

    [[nodiscard]] const std::string& source() const noexcept { return data_.source; }
    [[nodiscard]] const std::string& source_id() const noexcept { return data_.source_id; }
    [[nodiscard]] const std::string& reference() const noexcept { return data_.reference; }
    [[nodiscard]] const std::optional<std::string>& title() const noexcept { return data_.title; }
    [[nodiscard]] const std::optional<std::string>& description() const noexcept {
        return data_.description;
    }
    [[nodiscard]] const std::optional<AskingPrice>& asking_price() const noexcept {
        return data_.asking_price;
    }
    [[nodiscard]] const std::optional<std::string>& location() const noexcept {
        return data_.location;
    }
    [[nodiscard]] const ShippingDetails& shipping() const noexcept { return data_.shipping; }
    [[nodiscard]] const std::vector<std::string>& image_references() const noexcept {
        return data_.image_references;
    }
    [[nodiscard]] const std::optional<std::string>& seller_condition() const noexcept {
        return data_.seller_condition;
    }
    [[nodiscard]] CaptureTime captured_at() const noexcept { return captured_at_; }

  private:
    ListingData data_;
    CaptureTime captured_at_;
};

} // namespace sniff

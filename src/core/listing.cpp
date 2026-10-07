#include "core/listing.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace sniff {
namespace {

bool is_blank(std::string_view text) {
    return text.find_first_not_of(" \t\n\r\f\v") == std::string_view::npos;
}

void require_reference(std::string_view value, std::string_view field) {
    if (is_blank(value)) {
        throw std::invalid_argument(std::string(field) + " must not be blank");
    }
}

} // namespace

Listing::Listing(ListingData data, CaptureTime captured_at)
    : data_(std::move(data)), captured_at_(captured_at) {
    require_reference(data_.source, "source");
    require_reference(data_.source_id, "source_id");
    require_reference(data_.reference, "reference");

    if (data_.asking_price) {
        const auto& price = *data_.asking_price;
        if (price.minor_units < 0) {
            throw std::invalid_argument("asking price must not be negative");
        }
        // This checks code syntax, not membership in a currency registry.
        if (price.currency.size() != 3 ||
            !std::all_of(price.currency.begin(), price.currency.end(),
                         [](char character) { return character >= 'A' && character <= 'Z'; })) {
            throw std::invalid_argument("currency must be a three-letter uppercase code");
        }
    }

    for (const auto& reference : data_.image_references) {
        require_reference(reference, "image reference");
    }
}

} // namespace sniff

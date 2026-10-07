#pragma once

#include "core/listing.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

namespace sniff::persistence {

class ListingDatabase {
  public:
    // Opens or creates a persistent file and initializes schema version 1.
    explicit ListingDatabase(const std::filesystem::path& path);
    ~ListingDatabase();

    ListingDatabase(const ListingDatabase&) = delete;
    ListingDatabase& operator=(const ListingDatabase&) = delete;

    // Inserts an entire snapshot atomically; rejects duplicate source/ID/capture times.
    [[nodiscard]] std::int64_t save(const Listing& listing);
    [[nodiscard]] std::optional<Listing> load(std::int64_t snapshot_id) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sniff::persistence

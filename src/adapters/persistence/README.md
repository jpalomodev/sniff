# Listing Persistence

## Connection

`sniff::persistence::ListingDatabase` opens or creates a file-backed SQLite database.
The connection is owned by a private implementation and closed automatically on destruction.
Copying is disabled; use one instance per thread and do not share an instance concurrently.
Parent directories are created as needed. Empty paths, `:memory:`, and embedded NUL paths
are rejected. SQLite URI parsing is not enabled.

Foreign keys are enabled, synchronous writes use FULL, and lock contention waits for up to
five seconds before reporting an error. SQLite's rollback journal is retained; WAL is not
needed for the current single-operator workflow. Committed records survive closing and
reopening the connection. Open, SQL, and schema errors are exceptions rather than fabricated
results; filesystem errors also propagate.

## Schema

`schema.sql` is embedded at CMake configure time, so startup does not depend on the current
working directory finding a SQL file. Initial creation runs in a transaction and sets
`PRAGMA user_version = 1`. Opening version 1 leaves existing records intact. Other versions
and nonempty unversioned databases are rejected; future migrations must be explicit.

`listings` contains:

| Column | Meaning |
|---|---|
| `id` | Database snapshot ID, separate from the source's ID |
| `source`, `source_id`, `reference` | Required source identity and traceability |
| `title`, `description` | Nullable, original seller text |
| `asking_price_minor_units`, `currency` | Both NULL, or a nonnegative integer and uppercase three-letter code |
| `location` | Nullable source-provided location |
| `shipping_available` | NULL for unknown, 0 for unavailable, 1 for available |
| `shipping_terms` | Nullable seller-provided terms |
| `seller_condition` | Nullable original condition statement |
| `captured_at_ms` | Required integer milliseconds since the Unix epoch |

`listing_images` contains `listing_id`, zero-based `position`, and `reference`. A foreign
key links each image to its snapshot, and `(listing_id, position)` is the primary key.
Order and repeated image references are preserved. No image rows means no images captured.
Image bytes remain outside this model.

Both tables are STRICT, requiring SQLite 3.37 or newer (satisfied by the pinned dependency).
Constraints mirror the Listing rules. A unique `(source, source_id, captured_at_ms)` key
rejects duplicate captures, while different timestamps preserve source listing history.
Two distinct captures of the same listing within the same millisecond currently collide;
this is an explicit initial identity assumption that can change with collection requirements.

## API

```cpp
#include "adapters/persistence/listing_database.hpp"

sniff::persistence::ListingDatabase database{"data/sniff.db"};
const auto snapshot_id = database.save(listing);
const auto restored = database.load(snapshot_id); // std::optional<sniff::Listing>
```

`save()` binds parameters and inserts the listing and all images in one transaction. A
failure rolls everything back. It returns a signed 64-bit snapshot ID only after commit.
`load()` reads the row and ordered images in one read transaction, reconstructs a validated
Listing, and returns `std::nullopt` only when the ID is absent. SQL failures throw.

No update/delete or valuation logic is included. A subsequent collection saves another
snapshot rather than overwriting observed evidence. Lookup by source, history queries, and
repositories for other entities can be added as workflows require them.

## CLI

From the persistence worktree:

```bash
./build/debug/sniff init-db --path data/sniff.db
```

The default path is `data/sniff.db`, relative to the process's working directory. This command
creates the database if needed and validates schema compatibility on later invocations.
It does not collect or insert listings. Database files remain Git-ignored.

## Verification

Offline tests cover reopen persistence, exact field and image round-trips, unknown versus
zero/empty values, duplicate rejection, capture history, rollback after an image-insert
failure, schema constraints, and invalid files/versions/paths.

The Listing implementation and tests were copied unchanged from the core-domain worktree
because that work is not committed yet. Coordinate their integration before merging branches.

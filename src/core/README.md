# Core Logic

## Intended Responsibilities

Add domain types and deterministic business logic here as workflows are implemented:
listings, identity and evidence, normalization, comparables, valuation, repair economics,
opportunity scoring, and transaction outcomes.

Keep identification separate from valuation. Preserve unknown values, source references,
and the distinction between observations and inference. Financial calculations belong
here rather than in CLI code or model prompts.

The `sniff_core` library currently provides `Listing` in `core/listing.hpp`. Module names
and boundaries can change as the requirements develop.

## Listing Snapshot

Construct `Listing` with a `ListingData` input and an explicit `CaptureTime` (system-clock
time at millisecond precision). It owns its data and exposes const accessors without
field setters. Capture time is supplied by the collector so replay does not replace it
with the current time. A later collection creates a new snapshot.

Source, source ID, and reference must contain non-whitespace characters. A reference
may be a URL or another traceable source reference. IDs stay strings to preserve leading
zeros and nonnumeric identifiers. Callers identify a source listing by source plus ID;
the class does not deduplicate listings.

Title, description, asking price, location, and seller condition are optional. Missing
data is represented by `std::nullopt`; observed text is preserved without trimming or
normalizing it. Shipping availability is an optional bool, with optional seller terms.
Image references may be remote URLs or local references; an empty vector means no images
were captured, not proof that the original listing has no images. Blank image references
are rejected.

`AskingPrice` stores a nonnegative signed 64-bit minor-unit amount and a three-letter
uppercase currency code. For USD, 12345 minor units means $123.45; for JPY, 12345 means
12345 yen. Adapters must resolve currency, scale, and ambiguous price text before constructing
a price. A missing price remains unknown, while zero is an explicit asking price. Currency
validation checks syntax only; currency registry validation and currency conversion are
outside this class. No financial arithmetic is implemented yet.

Invalid traceability fields, prices, and image references throw `std::invalid_argument`.
Malformed source payloads should remain available in adapter storage even if they cannot
be represented as a validated Listing. No inferred identity, market value, repair advice,
or purchase recommendation is stored in Listing.

CREATE TABLE listings (
    id INTEGER PRIMARY KEY,
    source TEXT NOT NULL CHECK (length(trim(source, char(9,10,11,12,13,32))) > 0),
    source_id TEXT NOT NULL CHECK (length(trim(source_id, char(9,10,11,12,13,32))) > 0),
    reference TEXT NOT NULL CHECK (length(trim(reference, char(9,10,11,12,13,32))) > 0),
    title TEXT,
    description TEXT,
    asking_price_minor_units INTEGER CHECK (asking_price_minor_units >= 0),
    currency TEXT CHECK (length(currency) = 3 AND currency NOT GLOB '*[^A-Z]*'),
    location TEXT,
    shipping_available INTEGER CHECK (shipping_available IN (0, 1)),
    shipping_terms TEXT,
    seller_condition TEXT,
    captured_at_ms INTEGER NOT NULL,
    CHECK ((asking_price_minor_units IS NULL) = (currency IS NULL)),
    UNIQUE (source, source_id, captured_at_ms)
) STRICT;

CREATE TABLE listing_images (
    listing_id INTEGER NOT NULL REFERENCES listings(id) ON DELETE CASCADE,
    position INTEGER NOT NULL CHECK (position >= 0),
    reference TEXT NOT NULL CHECK (length(trim(reference, char(9,10,11,12,13,32))) > 0),
    PRIMARY KEY (listing_id, position)
) STRICT;

PRAGMA user_version = 1;

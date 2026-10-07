# External Adapters

## Intended Responsibilities

SQLite listing persistence is implemented in [persistence/](persistence/README.md). Add
libcurl transport, direct HTTP/JSON AI clients, and marketplace collectors as needed.
Keep vendor-specific formats and errors behind small interfaces
that serve core workflows. Validate model output before passing it to business logic.

Integrations must handle missing data, timeouts, access constraints, and partial failures.
Preserve source evidence where practical and keep credentials out of logs and ordinary
configuration. The first marketplace and AI provider remain undecided.

If browser automation becomes necessary, add a small TypeScript/Playwright sidecar under
`workers/` with a documented, versioned JSON contract. No Node runtime is required by this
scaffold.

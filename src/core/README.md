# Core Logic

## Intended Responsibilities

Add domain types and deterministic business logic here as workflows are implemented:
listings, identity and evidence, normalization, comparables, valuation, repair economics,
opportunity scoring, and transaction outcomes.

Keep identification separate from valuation. Preserve unknown values, source references,
and the distinction between observations and inference. Financial calculations belong
here rather than in CLI code or model prompts.

Create a compiled core library when the first behavior needs one. Module names and
boundaries can change as the requirements develop.

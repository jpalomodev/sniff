# AGENTS.md

## Mission

Build a private, operator-focused resale intelligence tool for identifying underpriced, parcel-shippable professional electronics.

The product exists to improve real purchasing decisions and realized resale profit.

Do not optimize for demo appeal, generic AI features, or SaaS readiness.

## Product Scope

The MVP should support this pipeline:

`discover -> identify -> gather evidence -> value -> evaluate -> rank -> review -> record outcome`

Initial vertical:
- portable professional/test-and-measurement electronics

The design should allow later verticals without making the MVP generic prematurely.

Possible future verticals:
- industrial automation electronics
- telecom/fiber test gear
- automotive diagnostic tools
- marine electronics
- survey/GNSS equipment
- other specialized professional electronics

## Core Product Rules

1. Sold-market evidence is more important than asking prices.
2. Never represent an asking price as proven market value.
3. Preserve uncertainty.
4. Prefer "unknown" over fabricated precision.
5. Separate observed facts from model inference.
6. Keep source evidence traceable.
7. The operator always makes the purchase decision.
8. Optimize for realized profit and capital turnover, not headline ROI alone.
9. Broken/untested items must include downside and repair-failure scenarios.
10. Hazardous repair work must be flagged rather than casually instructed.

## Engineering Principles

- Keep the system simple enough for one developer to understand.
- Prefer explicit modules and data flow over clever abstractions.
- Avoid speculative architecture.
- Avoid adding infrastructure that the current MVP does not need.
- Make important business rules configurable where practical.
- Do not bury business logic inside presentation code.
- Keep marketplace-specific code behind clear interfaces.
- Keep valuation logic separate from discovery logic.
- Keep AI/model integrations behind replaceable adapters.
- Persist raw source data when practical so analysis can be reproduced.
- Design for imperfect external data and partial failures.

## Suggested Domain Boundaries

Use these as conceptual modules, not mandatory class/file names:

- `discovery`
- `listing`
- `media`
- `identification`
- `normalization`
- `comparables`
- `valuation`
- `repair_analysis`
- `opportunity_scoring`
- `persistence`
- `cli`
- `outcomes`

Avoid coupling modules directly to one another when a small domain interface is sufficient.

## Data Requirements

A listing should preserve:
- source
- source identifier
- URL/reference
- title
- description
- asking price
- location/shipping metadata when available
- image references
- seller-stated condition
- capture timestamp

An identified item should support:
- manufacturer
- family
- model
- part number
- configuration/options
- condition interpretation
- identification confidence
- evidence supporting the identification

A valuation should support:
- resale range
- evidence/comparables
- fees/cost assumptions
- liquidity estimate
- uncertainty/confidence
- downside case

An opportunity should support:
- expected economics
- major risks
- missing information
- rank/score
- concise rationale

A completed transaction should preserve actual outcomes so estimates can later be evaluated.

## AI Usage

Use AI where it provides leverage:
- extracting information from messy listings
- identifying equipment from text/images
- normalizing model names
- interpreting configuration
- summarizing comparable evidence
- reasoning about repair symptoms
- explaining uncertainty

Do not use an LLM where deterministic code is more reliable:
- arithmetic
- fee calculations
- sorting
- deduplication
- persistence
- validation
- exact filtering
- known business rules

Structured model outputs should be schema-validated before entering core business logic.

Treat model output as untrusted input.

## Opportunity Evaluation

Do not hard-code a simplistic rule such as:

`resale_price - asking_price > X`

Evaluation should be able to consider:
- acquisition price
- realistic resale range
- transaction fees
- shipping/packing
- repair cost
- repair probability
- salvage value
- return risk
- liquidity
- expected holding time
- confidence

Exact weights and thresholds are intentionally unspecified for now.

Make them easy to tune as real transaction data becomes available.

## Repair Analysis

Repair functionality is decision support, not blind instruction.

Prefer:
- likely failure modes
- diagnostic hypotheses
- service-manual references
- parts availability
- estimated difficulty
- expected repair economics
- safe verification steps

Flag equipment involving meaningful electrical, stored-energy, RF, laser, battery, or other hazards.

Never increase a buy recommendation merely because an AI-generated repair path sounds plausible.

## CLI

A CLI is sufficient for the MVP.

Favor commands that map cleanly to workflows, for example:

- discover listings
- inspect a listing
- analyze/identify
- show comparable evidence
- evaluate an opportunity
- record a purchase
- record a sale/outcome

Exact command names are flexible.

CLI output should be concise and decision-oriented.

## Testing

Tests should focus on behavior that could cost money if wrong.

Prioritize:
- parsing/normalization
- money arithmetic
- fee calculations
- valuation aggregation
- ranking behavior
- persistence
- malformed model output
- missing data
- duplicate listings
- incorrect source data
- failure/retry paths

Use fixtures for external services where possible.

Do not make ordinary test runs depend on live marketplaces or live model APIs.

## External Integrations

For every external source:
- isolate integration-specific code
- handle timeouts/errors explicitly
- respect source terms and access constraints
- avoid assuming fields are always present
- preserve enough raw data for debugging
- add rate limiting/retry behavior where appropriate

Do not silently convert an integration failure into fabricated data.

## Change Discipline

Before implementing a large change:

1. identify which MVP requirement it serves,
2. choose the smallest implementation that satisfies it,
3. avoid unrelated refactors,
4. add or update relevant tests,
5. update documentation when behavior changes.

If requirements are ambiguous, prefer a reversible implementation and document the assumption.

Do not invent detailed business thresholds simply to make the code feel complete.

## Coding Style

Follow the established project language/toolchain once present.

Until then:
- favor clear names
- keep functions focused
- avoid premature generic frameworks
- minimize global state
- make ownership/lifetimes explicit
- handle errors intentionally
- keep configuration separate from code where useful
- write comments for rationale, not obvious syntax

Do not introduce a new dependency unless it clearly reduces complexity or enables an MVP requirement.

## Definition of Done

A feature is done when:
- it supports a real MVP workflow,
- failure states are handled,
- important behavior is tested,
- outputs expose uncertainty where relevant,
- no fake data or placeholder logic is presented as real analysis,
- the implementation remains understandable to the primary developer.

## Current Development Bias

When forced to choose between:
- breadth vs depth -> choose depth
- automation vs correctness -> choose correctness
- polished UI vs useful analysis -> choose useful analysis
- complex architecture vs changeability -> choose changeability
- model confidence vs source evidence -> choose source evidence

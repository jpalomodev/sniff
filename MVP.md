# Private Resale Intelligence Engine — MVP

## 1. Purpose

Build a private tool that identifies underpriced, parcel-shippable professional electronics that can be resold for profit.

The MVP is for a single operator. It is not a public SaaS product.

The initial focus is portable professional/test-and-measurement electronics, with room to expand into adjacent categories such as industrial automation, telecom/fiber, automotive diagnostics, marine electronics, and other specialized equipment.

## 2. Core Thesis

The best opportunities occur when:

- the item has meaningful secondary-market demand,
- the seller does not fully understand its value,
- the item is difficult for a regular owner to identify or price,
- it is reasonably easy to store and ship,
- model/configuration details materially affect value,
- condition, completeness, repairability, or part-out value create additional pricing gaps.

The system should optimize for **realized profit and capital turnover**, not impressive-looking asking-price spreads.

## 3. MVP Workflow

### A. Discover
Collect candidate listings from one supported marketplace/source.

Capture enough information to analyze each listing:

- title
- description
- asking price
- location/shipping information
- photos
- listing URL/reference
- seller-provided condition

Discovery should be modular so additional sources can be added later.

### B. Identify
Infer the most likely item identity from listing text and images.

Prefer exact identification where possible:

- manufacturer
- product family
- model
- part number
- relevant configuration/options

Preserve uncertainty rather than forcing an identification.

### C. Value
Estimate a realistic resale range using available comparable sales and market evidence.

The valuation should distinguish:

- asking prices from sold prices
- complete vs incomplete configurations
- working vs untested vs broken condition
- relevant accessories/options
- likely liquidity

Confidence should be surfaced explicitly.

### D. Evaluate
Estimate whether the listing is worth pursuing.

Consider, without hard-coding premature thresholds:

- acquisition cost
- estimated resale value
- marketplace fees
- shipping/packing
- likely repair cost
- expected holding period
- downside/salvage value
- uncertainty
- probability of successful resale
- expected net profit
- capital efficiency

Output a ranked opportunity score plus a short explanation.

### E. Repair / Part-Out Analysis
For broken, untested, or incomplete items, estimate:

- likely failure modes
- repair difficulty
- expected parts cost
- probability of successful repair
- likely value if repaired
- salvage / part-out value if repair fails
- whether part-out may be preferable to repair

Repair guidance should remain conservative around hazardous equipment and should flag work that is unsuitable for casual DIY repair.

### F. Review
Present opportunities through a simple operator-facing interface.

For the MVP, this may be a CLI.

A useful result should make it easy to answer:

> What is this, what is it probably worth, how confident are we, what could go wrong, and should I investigate it further?

## 4. Human-in-the-Loop Principle

The MVP is an intelligence and prioritization system, not an autonomous purchasing bot.

It should:

- surface evidence,
- explain uncertainty,
- flag missing information,
- recommend manual verification when needed,
- never treat a single asking price as proof of market value.

The operator makes all purchase decisions.

## 5. Data Model

At minimum, persist:

### Listing
Source listing data and images.

### Item Identity
Normalized manufacturer/model/part/configuration with confidence.

### Comparable
Observed market evidence used for valuation.

### Opportunity
Computed economics, risk, confidence, and ranking.

### Transaction Outcome
For items actually purchased:

- purchase price
- repair/parts cost
- listing/sale price
- fees
- shipping
- final profit/loss
- holding time
- repair result
- return/refund outcome

Transaction outcomes should eventually become proprietary training/decision data for the system.

## 6. MVP Success Criteria

Do not lock the project to arbitrary numeric targets during initial development.

The MVP is successful if it can repeatedly:

1. discover plausible resale opportunities,
2. correctly identify enough specialized items to be useful,
3. estimate value using credible evidence,
4. reject misleading or weak opportunities,
5. rank opportunities well enough to guide manual sourcing,
6. record real-world outcomes so predictions can improve.

The ultimate validation is **realized profit from completed transactions**, not simulated ROI.

## 7. Explicit Non-Goals

The MVP does **not** need:

- a polished GUI
- multi-user support
- subscriptions or billing
- public accounts/authentication
- automated purchasing
- inventory warehouse management
- support for every marketplace
- support for every electronics category
- perfect repair diagnosis
- perfect computer vision
- sophisticated ML trained from scratch
- enterprise-scale infrastructure

Prefer a narrow system that produces useful decisions over a broad system that looks impressive.

## 8. Development Priorities

Prioritize in this order:

1. correctness of data
2. trustworthy valuation evidence
3. useful ranking
4. clear uncertainty
5. maintainable architecture
6. speed and automation
7. UI polish

The project should remain easy to change as real transaction data reveals which assumptions are wrong.

## 9. Expansion Path

After the MVP proves useful:

- add additional marketplaces
- improve image-based identification
- add demand/supply monitoring
- improve repair knowledge
- add part-out valuation
- learn from completed transactions
- expand into adjacent equipment verticals
- identify repeat suppliers and business liquidations
- automate more of the sourcing workflow

The long-term advantage should come from proprietary transaction history, repair outcomes, valuation knowledge, and cross-market observations—not merely from access to an LLM.

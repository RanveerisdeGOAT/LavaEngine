# LavaEngine — Current Issues & Work Tracker

> **Snapshot:** 2026-09-17  
> **Repository:** https://github.com/RanveerisdeGOAT/LavaEngine  
> **Version observed:** `0.7.1-indev`
>
> This file tracks issues, gaps, and engineering work identified from the current repository structure, README, changelog, and public project state.
>
> **Important:** Items marked as architectural/feature gaps are not necessarily bugs. They are things that should be clarified, implemented, tested, or documented as LavaEngine matures.

---

## Status Legend

- `[ ]` Open
- `[~]` In progress / partially implemented
- `[x]` Complete
- `[?]` Needs verification

## Priority Legend

- **P0 — Critical:** blocks reliable use of the framework
- **P1 — High:** important for the next usable milestone
- **P2 — Medium:** important for robustness/usability
- **P3 — Low:** polish, convenience, or long-term work
- **IDX** issue id.

---

## [~] ID2 - Define and enforce ownership/lifetime rules

**Priority:** P0  
**Area:** Core / Containers / Modules / Resources

The 0.7.1 changelog says that several lifetime issues were fixed, but the architecture relies heavily on Containers, Modules, Resources, shared references/handles, and cross-container access.

We need a documented and tested ownership model covering:

- Container destruction
- Module destruction
- Resource destruction
- Resource handles after owner destruction
- Imported/exported resources
- References held by scheduled jobs
- References held by Inspector code
- References held by events
- Cross-container resource access
- Shutdown ordering

### Acceptance criteria

- No use-after-free in core lifecycle paths.
- Resources have an unambiguous owner/lifetime policy.
- Jobs cannot outlive objects they capture unless explicitly supported.
- Destruction order is deterministic and documented.
- Regression tests cover lifetime edge cases.

---

## [ ] ID1 - Define scheduler dependency and execution semantics

**Priority:** P0  
**Area:** Scheduler

The README establishes the scheduler as a fundamental part of LavaEngine, but the public design does not yet fully specify:

- Job ordering
- Dependencies between jobs
- Whether jobs may execute concurrently
- Thread affinity
- Cancellation
- Error propagation
- Shutdown behavior
- Re-entrancy
- Long-running jobs
- Resource lifetime while jobs are running

The current scheduler example also uses return values to determine whether a job repeats. This needs a precise contract.

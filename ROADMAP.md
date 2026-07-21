# SWC-DB Development Roadmap

Strategic phases for further development of the SWC-DB monorepo (C++20 distributed wide-column DBMS; source version `0.5.13`).

This document is the **product plan**. Detailed scored backlogs and P0/P1 risk items live in [`evals/`](evals/) — especially [`evals/QUALITY_EVALUATION_REPORT.md`](evals/QUALITY_EVALUATION_REPORT.md). Day-to-day agent policy lives in [`.cursor/rules/`](.cursor/rules/).

**Ordering principle:** trust before features; measure before optimize; release/docs sync at each shippable gate. Performance, Features, Stability, and Usability/Readability are woven into each phase where they apply — not four disconnected tracks.

```mermaid
flowchart LR
  P1[Phase1_Trust]
  P2[Phase2_Verification]
  P3[Phase3_ReleaseUX]
  P4[Phase4_Performance]
  P5[Phase5_ProductScale]
  P1 --> P2 --> P3 --> P4 --> P5
```

**Baseline (2026-07 audits):** Quality **2.6/5**, Docs **3.4/5**, Standards **3.9/5** — see [`evals/README.md`](evals/README.md).

Feature ideas from the community: GitHub issues with the [`enhancement`](https://github.com/kashirin-alex/swc-db/labels/enhancement) label.

---

## Phase 1 — Trust & Integrity (Stability first)

**Goal:** Make untrusted peers and corrupted on-disk data fail safely. Highest severity, smallest diffs.

| Concern | Work |
|---------|------|
| **Stability** | Cap `header_len` vs `Header::MAX_LENGTH`; bound payload before `reallocate()`; honor connect timeout; null-check `m_metrics` on `ev->error`; log unexpected `catch(...)` in [`ConnHandler.cc`](src/cc/lib/swcdb/core/comm/ConnHandler.cc) |
| **Stability** | Fail-fast on truncated CellStore/CommitLog cells; sanity-cap CellStore index metadata; harden compaction `apply_new` ordering in ranger storage |
| **Usability** | Document the fail-fast corruption policy in operator docs (what operators see / how to recover) |
| **Performance / Features** | Out of scope except avoiding regressions on the touched paths |

**Detailed backlog:** Quality eval §4 (P0/P1 register) — items R-P0-1 through R-P1-3 in [`evals/QUALITY_EVALUATION_REPORT.md`](evals/QUALITY_EVALUATION_REPORT.md).

**Exit criteria:** Quality P0 items closed; truncated-cell and compaction apply risks addressed or explicitly deferred with tests; quality re-score target ≥ **3.2** on security/correctness dimensions.

**Primary paths:** `src/cc/lib/swcdb/core/comm/`, `src/cc/lib/swcdb/ranger/db/{CommitLogFragment,RangeBlock,CellStore,RangeBlocks}.cc`, daemon `AppContext.h` files.

---

## Phase 2 — Verification Backbone (Stability + Usability of engineering)

**Goal:** End CI false confidence so every later phase can ship without guesswork.

| Concern | Work |
|---------|------|
| **Stability** | Unit tests on every PR (narrow or remove `[TEST COMMIT]` gate for the unit job); scheduled/nightly `TEST=2` integration including broker; one ASan job |
| **Stability** | Add/restore compaction + CommitLogCompact + Ranger protocol coverage; keep FS insistence retries as intentional design |
| **Usability** | Update [`CONTRIBUTING.md`](CONTRIBUTING.md) CI truth table to match new gates; document “what CI covers vs local cluster” |
| **Performance** | Smoke latency/throughput check via `swcdb_load_generator` in nightly (not yet a full perf suite) |
| **Features** | Binding smoke builds (Python uncomment tests, Java/Ruby minimal) so language features do not rot |

**Detailed backlog:** Quality eval §3.5–3.6, §5.2–5.3 (test/CI gaps); [`CONTRIBUTING.md`](CONTRIBUTING.md) “What CI Runs Today”.

**Exit criteria:** Default PR CI runs unit tests; nightly runs integration; compaction path has at least one automated test; bindings compile in CI on a subset.

**Primary paths:** [`.github/workflows/ci.yml`](.github/workflows/ci.yml), [`tests/integration/`](tests/integration/), [`CONTRIBUTING.md`](CONTRIBUTING.md).

---

## Phase 3 — Release & Operator Clarity (Usability / Readability)

**Goal:** Ship a coherent 0.5.13+ release experience and close the journey gaps that block adoption.

| Concern | Work |
|---------|------|
| **Usability** | Sync packages + docs: `docs/install/getting_swcdb/`, `swc.install.archive`, Arch/Debian packaging, [`CHANGELOG.md`](CHANGELOG.md) for unreleased master features (OR scan, SQL `\|\|`, dependency bumps) |
| **Usability** | Expand thin admin CLI guides ([`docs/use/cli/manager/`](docs/use/cli/), ranger, filesystem); bring C++ client guide closer to Thrift depth |
| **Readability** | Optional docs link-check CI; keep redirect stubs `nav_exclude` |
| **Features** | Finish small incomplete surfaces that block release polish: `SpecsUpdateOp` ERASE TODO, Python Thrift TLS stub, ThriftBroker SSL wiring comments |
| **Stability** | Re-run docs integrity checklist from [`evals/DOCS_CLARITY_EVALUATION.md`](evals/DOCS_CLARITY_EVALUATION.md); no behavior changes beyond Phase 1/2 leftovers |

**Detailed backlog:** Docs eval backlog items #1, #11, #12 in [`evals/DOCS_CLARITY_EVALUATION.md`](evals/DOCS_CLARITY_EVALUATION.md).

**Exit criteria:** Tagged release with matching docs/packages; docs score target ≥ **3.8**; Getting Started path works against the published archive without version drift.

---

## Phase 4 — Measured Performance

**Goal:** Replace the historical v0.4.9 claim with a repeatable baseline, then optimize hot paths with evidence.

| Concern | Work |
|---------|------|
| **Performance** | Formalize a small benchmark matrix (local FS + one remote FS): write/select/compaction under `swcdb_load_generator`; publish numbers under `docs/configure/benchmarks/` |
| **Performance** | Profile and tune hot paths: ranger `Range` / `CompactRange`, client `QuerySelect`, wire encoding defaults; optional TCMalloc release guidance with numbers |
| **Readability** | Split the worst god-files on natural seams when touching them for perf (`CompactRange.cc`, `Range.cc`, `load_generator.cc`) — prefer extract-as-you-touch over big-bang refactor |
| **Stability** | Keep ASan/TSan in the loop for any concurrent hot-path change |
| **Features** | No large new product surface in this phase |

**Detailed backlog:** Standards eval S1 (god files) in [`evals/STANDARDS_CLARITY_EVALUATION.md`](evals/STANDARDS_CLARITY_EVALUATION.md); quality eval §3.2 (files >1000 LOC).

**Exit criteria:** Documented current-version benchmarks; at least one measurable win on a named workload; no new P0/P1 from the work.

---

## Phase 5 — Product Depth & Maintainability (Features + long-term Readability)

**Goal:** Grow capability where the product checklist is explicitly weak, and reduce structural debt that blocks every future change.

| Concern | Work |
|---------|------|
| **Features** | Prioritize from GitHub `enhancement` issues plus product gaps: access controls / multi-tenant auth, Thrift SSL end-to-end, remaining `NOT_IMPLEMENTED` protocol handlers, experimental `CELL_DEFINED` decision (ship or remove), SQL surface gaps (`limit_by`/`offset_by` stubs) — **not** full relational joins (explicitly out of product model unless strategy changes) |
| **Features** | Manager HA / failover story: either implement redundancy or document a supported ops pattern (today coordination is single-manager; replication is FS-dependent) |
| **Readability** | Shared daemon bootstrap / metrics scaffolding; align manager/ranger dispatch to table style when adding commands; long-term reduce `.cc`-in-header aggregation where it eases review |
| **Stability** | Protocol and auth changes behind tests from Phase 2; preserve intentional FS insistence and CellStore rename-rollback patterns |
| **Performance** | Any new feature path gets a load-generator scenario before claiming readiness |

**Detailed backlog:** Quality eval §5.1, R-P3 items; standards eval S1, S6 in [`evals/STANDARDS_CLARITY_EVALUATION.md`](evals/STANDARDS_CLARITY_EVALUATION.md).

**Exit criteria:** Chosen feature set shipped with tests + docs; god-file and daemon-duplication debt measurably reduced on the touched modules; standards readability dimension trending up from **2.8**.

---

## How the four concerns map across phases

| | Phase 1 | Phase 2 | Phase 3 | Phase 4 | Phase 5 |
|--|---------|---------|---------|---------|---------|
| **Stability** | Primary | Primary | Support | Support | Support |
| **Usability / Readability** | Docs of policy | CI docs | Primary | Extract-as-you-touch | Structural cleanup |
| **Performance** | Guardrails only | Smoke | — | Primary | Feature-path budgets |
| **Features** | Blockers only | Bindings smoke | Small gaps | — | Primary |

---

## Related documents

| Document | Role |
|----------|------|
| [`evals/QUALITY_EVALUATION_REPORT.md`](evals/QUALITY_EVALUATION_REPORT.md) | P0–P3 risk register, CI truth table, test gaps |
| [`evals/DOCS_CLARITY_EVALUATION.md`](evals/DOCS_CLARITY_EVALUATION.md) | Docs backlog and integrity gates |
| [`evals/STANDARDS_CLARITY_EVALUATION.md`](evals/STANDARDS_CLARITY_EVALUATION.md) | Layer A/B standards, god-file and build-model debt |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | Human workflow, CI gates, local testing |
| [`AGENTS.md`](AGENTS.md) | Agent entry point and module map |

Refresh `evals/` reports on purpose when re-scoring a phase; update this roadmap when phase scope or ordering changes.

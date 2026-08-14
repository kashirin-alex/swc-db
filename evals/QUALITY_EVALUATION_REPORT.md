# SWC-DB Codebase Quality Evaluation Report

**Date:** 2026-07-12  
**Scope:** C++ core/db/fs/daemons, tests, CI, Thrift gateway, binding test status, docs gaps affecting quality process  
**Method:** Baseline inventory + line-level verification of risk register (Phases 1–4 of the quality evaluation plan)  
**Out of scope:** Implementing fixes, packaging/release, performance benchmarking

---

## 1. Executive summary

| | |
|--|--|
| **Overall score** | **2.7 / 5.0** (weighted) |
| **Maturity** | Build-strong, automation-weak; architecture intent clear, implementation modularity and test gates lag |
| **Top risks** | (1) Truncated on-disk cells continue, (2) CI false confidence, (3) metrics null-deref on `ev->error`, (4) connect timeout ignored, (5) empty `catch(...)` on receive |

SWC-DB has a coherent product topology (clients → broker/thrift → manager → ranger → FS), centralized wire protocol, strict compiler warnings-as-errors, and clean module layering at the `#include` level. Quality debt concentrates in: untrusted-input bounds on the native comm path, ranger on-disk integrity policy under corruption, opt-in CI (sparse `TEST=2` integration when gated), and daemon build aggregation that produces god-sized translation units.

**Recommendation:** Treat remaining P0 (empty `catch(...)`, connect timeout) and P1 truncated-cell / compaction apply-ordering as the next engineering fixes; in parallel, fix CI so every PR runs unit tests and a scheduled job runs integration.

---

## 2. Dimension scores

| Dimension | Weight | Score (1–5) | Weighted | Summary |
|-----------|--------|-------------|----------|---------|
| Correctness & reliability | 30% | **2.8** | 0.84 | Strong intentional patterns (CellStore rename-rollback; insistent FS I/O); remaining: truncated-cell continue, compaction apply window |
| Security & robustness | 25% | **2.0** | 0.50 | Score frozen at 2026-07-12 audit — re-score on next pass |
| Test & CI maturity | 20% | **2.5** | 0.50 | Capable local harness; CI opt-in with sparse `TEST=2` integration includes; compaction/Rgr protocol gaps |
| Architecture & modularity | 15% | **3.5** | 0.525 | Excellent layering and protocol centralization; weak compile-time modularity (`.cc`-in-header) |
| Maintainability | 10% | **3.0** | 0.30 | Consistent style, low TODO noise; god files and duplicated AppContext/metrics |
| **Overall** | 100% | | **2.665 ≈ 2.7** | |

---

## 3. Phase 1 — Baseline inventory

### 3.1 Module size map (`src/cc/`)

| Module | Files (include+lib) | LOC include | LOC lib | Role |
|--------|--------------------:|------------:|--------:|------|
| core | 88 | 11 635 | 6 532 | Foundation, comm, config |
| common | 10 | 3 111 | 0 | Shared header-only |
| db | 225 | 19 833 | 15 007 | Cells, protocol, client, SQL |
| fs | 106 | 5 455 | 5 838 | FS abstraction + backends |
| manager | 63 | 4 888 | 3 743 | Metadata / placement |
| ranger | 84 | 6 533 | 8 503 | Storage engine |
| broker | 11 | 1 192 | 38 | Client front-end |
| fsbroker | 24 | 1 604 | 81 | Remote FS RPC |
| thrift | 8 | 2 628 | 38 | Thrift gateway |
| utils | 15 | 1 049 | 2 530 | CLI / tools |

### 3.2 Files >1000 lines (god-file candidates)

| Lines | Path |
|------:|------|
| 1300 | `src/cc/include/swcdb/thrift/broker/AppHandler.h` |
| 1298 | `src/cc/lib/swcdb/ranger/db/Range.cc` |
| 1214 | `src/cc/lib/swcdb/db/client/sql/QuerySelect.cc` |
| 1138 | `src/cc/lib/swcdb/ranger/db/CompactRange.cc` |
| 1079 | `src/cc/lib/swcdb/core/config/Property.cc` |
| 1038 | `src/cc/bin/swcdb/utils/load_generator.cc` |

Additional files in the 600–999 range include `MngdColumns.cc` (998), `MetricsReporting.h` (947), `ConnHandler.cc` (791), `CommitLogFragment.cc` (746), `CommitLog.cc` (729), `CellStore.cc` (645), and all four FS backend implementations (~665–762).

### 3.3 Include-graph sample

| Check | Result |
|-------|--------|
| `core/` includes `db` / daemons | **None** (leaf layer) |
| manager ↔ ranger cross-includes | **None** |
| broker → manager/ranger headers | **None** |
| Coordination path | Shared `db/Protocol/*` + `db/client/*` |

### 3.4 `.cc`-in-header (daemon aggregation)

- **10 headers** under manager/ranger/broker/fsbroker/thrift pull `.cc` files
- **34** unconditional `#include "*.cc"` directives
- Hotspots: `ranger/db/Columns.h` (entire storage stack), `ranger/db/Compaction.h` (`CompactRange.cc`), `manager/MngrEnv.h` (role/rangers/columns)

### 3.5 Test inventory

| Area | Sources | Type |
|------|---------|------|
| `tests/libswcdb_core/` | 8 `.cc` + 1 `.golden` | Unit |
| `tests/libswcdb/` | 6 `.cc` | Unit (cells/keys/specs) |
| `tests/integration/` | 10 sources + shell | Cluster / in-process |
| Golden via `testdiff` | **1** file only | `properties_parser_test.golden` |

Client integration expands to **288** query permutations (`2×2×3×3×4×2` in `tests/integration/client/CMakeLists.txt`). Utils shell: 24 cases. Broker reuses manager column test via `#define CLIENT_EXECUTOR 2`.

### 3.6 CI truth table (`.github/workflows/ci.yml`)

| Fact | Evidence |
|------|----------|
| Opt-in gate | `gate` job checks head commit message for `[TEST COMMIT]` (PR head SHA or push tip); `main` needs `gate` |
| Matrix `TEST` | Default `[1]`; sparse `include` adds `TEST=2` (g++-13, O_LEVEL 3/OFF and 6/ON) |
| Unit tests | Run only when `TEST` is 1 or 2 **and** (`O_LEVEL=3`+`IMPL=OFF`) or (`O_LEVEL=6`+`IMPL=ON`) |
| Integration | Steps require `TEST == '2'` → run on the sparse includes above |
| Thrift | Default `0.20.0`; sparse `include` builds `THRIFT=0.23.0` with `WITHOUT_THRIFT_C=ON` |
| Broker integration | **No CI step** (CMake/target exists locally) |
| Languages | `-DSWC_LANGUAGES=NONE` |
| Ceph FS test | `-DSWC_SKIP_TEST_FS_CEPH=ON` |
| Sanitizers | Not configured in CI |
| Runner / compilers | `ubuntu-24.04`; g++-12/13/14, clang++-16/17/18 |
| Checkout action | `actions/checkout@v7` |

### 3.7 TODO density

| Pattern | Count in `src/cc` |
|---------|------------------:|
| `TODO` / `FIXME` / `HACK` / `XXX` | **1** (`SpecsUpdateOp.h`: `// + TODO: ERASE`) |

Technical debt appears mainly as commented-out code and large files, not inline markers.

---

## 4. Verified risk register

Classification: **Confirmed** / **Partial** / **False positive** / **Intentional design**.

### P0 — Security / crash

#### R-P0-3: Empty `catch(...)` on receive — **Partial (confirmed pattern, narrower than “silent close”)**

- Outer `catch(...) { }` at `ConnHandler.cc:331`, `364`, `403` swallows unexpected exceptions without logging, then `do_close_recv()`.
- Common truncation / `ec` paths **do** log `LOG_WARN` (lines 319, 355, 390).
- Sender callbacks also use empty `catch(...) { }` then `do_close()` (lines 117, 155).
- Conflicts with `.cursor/rules/cpp-error-logging-memory.mdc` for the unexpected-exception path.
- Elsewhere in `core/comm`, most catches correctly use `SWC_LOG_CURRENT_EXCEPTION`.

#### R-P0-4: Sync connect timeout ignored — **Confirmed**

- `SerializedClient::get_connection(..., timeout, ...)` does `(void) timeout;` (`SerializedClient.cc:39`).
- Retry sleep is hardcoded `3000` ms (lines 75–76), not the caller-supplied timeout.

---

### P1 — Data integrity / reliability

#### R-P1-1: Truncated on-disk cells logged and continued — **Confirmed**

| Site | Behavior |
|------|----------|
| `CommitLogFragment.cc:452–466` (`load_cells`) | `catch(...)` logs `"Cell trunclated"` ERROR; **no `err` set**; partial cells already added remain |
| `CommitLogFragment.cc:485–499` (`split`) | Same — split may proceed with partial data |
| `RangeBlock.cc:223–258` | Same — block load continues with partial `m_cells` |

Policy today: **log and continue**, not fail-fast. Corrupt fragments can pollute in-memory state, splits, and subsequent compaction.

#### R-P1-2: CellStore index size / count uncapped — **Confirmed**

- `idx_size_enc`, `idx_size_plain`, `blks_count` decoded from on-disk headers and used for `pread`, decode buffer size, `blocks.reserve`, and per-block loops (`CellStore.cc:174–238`).
- Checksums validate integrity of *claimed* lengths, but there is **no sanity cap** against schema `block_size` / `block_cells` / RAM. Corrupt-but-checksum-consistent (or attacker-controlled if FS is untrusted) metadata can force huge allocations or long loops.
- Load path retries on error until `chk_base` break (`CellStore.cc:241–246`) — persistent corruption can spin with WARN rather than fail fast.

#### R-P1-3: Compaction `apply_new` ordering — **Confirmed (window)**

```94:108:src/cc/lib/swcdb/ranger/db/RangeBlocks.cc
void Blocks::apply_new(int &err,
                       CellStore::Writers& w_cellstores,
                       CommitLog::Fragments::Vec& fragments_old) {
  ...
  cellstores.replace(err, w_cellstores);
  if(err)
    return;
  ...
  commitlog.remove(err, fragments_old);
}
```

If `cellstores.replace` succeeds and `commitlog.remove` fails, new CellStores are live while old log fragments may remain — metadata inconsistency window.  
**Positive:** `CellStoreReaders::replace` uses rename-to-bak / tmp-to-cs / rollback on error (`CellStoreReaders.cc:224–258`).

#### R-P1-4: `CompactRange::quit` partial cleanup — **Confirmed (limited)**

- `quit()` removes tmp writer/dir and marks cancelled (`CompactRange.cc:1113–1130`) but does not roll back a partially applied manager split beyond that cleanup. Split failure paths attempt cleanup elsewhere; worth fault-injection testing.

#### R-P1-5: FS Interface insistent sync retry (failure-tolerance) — **Confirmed as intentional design** (not a defect)

Insistent FS failure-tolerance so that when the filesystem goes down and is later fixed/booted, in-flight sync I/O continues from the **same operation state** without abandoning the call.

**Contract** (`Interface.cc:14–16`, `183–410`):

- Retry forever on transient/unexpected FS errors via `for(;;)` + `hold_delay()` (10 ms), logging WARN on each retry.
- Exit the loop on `Error::OK`, selected benign codes (e.g. `ENOENT` / `EEXIST` where applicable), or `Error::SERVER_SHUTTING_DOWN`.
- Goal: FS outage → repair/boot → same `Interface` call resumes and completes; ranger/client mid-op state is preserved.

**Layered timeouts (not a contradiction):**

- Broker backend uses finite RPC timeouts (`swc.fs.broker.timeout` default **120 000** ms) at the **transport** layer.
- Interface insistence above that is **desired**: a timed-out Broker attempt returns an error, Interface retries, and when FS/Broker recovers the original sync op proceeds.
- Ceph `setup_connection()` spinning while `!initialize()` (`Ceph/FileSystem.cc:86–94`) is the same insistent category (wait until the cluster is usable).

Aligned with project “insistent” vocabulary elsewhere (`ClientConnQueue::insistent()`, `SWC_MALLOC_NOT_INSISTENT`). **Do not “fix” by capping Interface retries.**

---

### P2 — Process / false confidence

#### R-P2-1: CI opt-in; integration only on sparse includes — **Partial** (addressed)

See §3.6. Default matrix remains compile + narrow unit subset. Sparse `include` entries now set `TEST=2` (and `THRIFT=0.23.0`) when `[TEST COMMIT]` opens the gate; broker integration step and non-opt-in PR CI remain open.

#### R-P2-2: Compaction / Ranger protocol / non-local FS untested — **Confirmed**

| Area | Status |
|------|--------|
| `CompactRange.cc` / `CommitLogCompact.cc` | No dedicated tests; commitlog compact path commented in `test_commitlog.cc:68` |
| `Protocol/Rgr` (CellsSelect/Update, RangeLoad, …) | **No** test references under `tests/` |
| Ceph / Hadoop FS | CI skips Ceph; no hadoop test refs; ranger tests hardcode `--swc.fs=local` |
| SQL / client query | Integration (288 cases); sparse `TEST=2` CI includes |

#### R-P2-3: `m_metrics` null deref on `ev->error` — **Confirmed**

On the `ev->error` path, all four native AppContexts call `m_metrics->net->error(conn)` **without** a null check, while other paths use `if(m_metrics)`:

| File | Unchecked line |
|------|----------------|
| `manager/AppContext.h` | 136 |
| `ranger/AppContext.h` | 151 |
| `broker/AppContext.h` | 156 |
| `fsbroker/AppContext.h` | 185 |

Latent crash when metrics are disabled (`m_metrics == nullptr`).

---

### P3 — Maintainability

#### R-P3-1: Header-aggregated daemon build — **Confirmed**

Intentional per `.cursor/rules/build-impl-source.mdc`, but produces mega-TUs and poor incremental compile/test isolation. 34 `.cc` includes from Env/Columns/Compaction headers.

#### R-P3-2: God files — **Confirmed**

See §3.2. Ranger `db/` cluster (~8k LOC across Range/Compact/CommitLog/CellStore) is the highest-coupling hotspot.

#### R-P3-3: Duplicated daemon bootstrap / metrics — **Confirmed**

- `main.cc` for manager/ranger/broker nearly identical (~72 lines each).
- MetricsReporting: 1 shared header + 5 per-daemon wrapper pairs under `{manager,ranger,broker,fsbroker,thrift}/queries/update/`.
- Dispatch: broker/fsbroker use handler **tables**; manager/ranger use large **switches**.

#### R-P3-4: Typo debt — **Noted (low)**

- `RangeUnoadForMerge.h` (“Unoad”) used from manager health-check path — API confusion risk if renamed carelessly.

---

## 5. Phase 3 — Architecture, tests, bindings scorecards

### 5.1 Architecture (score 3.5 / 5)

| Criterion | Score | Evidence |
|-----------|------:|----------|
| Layering core → db/fs → daemons | 5 | No upward/cross-daemon includes |
| Protocol centralization | 5 | `db/Protocol/Commands.h`; handler/wire split |
| Compile-time modularity | 2 | Unconditional `.cc`-in-header for daemons |
| Dispatch consistency | 3 | Broker/fsbroker tables; manager/ranger switches |
| Env lifecycle consistency | 3 | Works, but naming/`IoCtx`/API differ per daemon |
| Duplication control | 2 | AppContext, metrics, mains copied |

**Strengths to preserve:** module map matching runtime roles; centralized commands; broker table-driven dispatch; CellStore rename-with-rollback; insistent FS Interface I/O; rich `Error::Code` taxonomy; `-Wall -Werror`.

### 5.2 Test & CI (score 2.5 / 5)

| Criterion | Score | Evidence |
|-----------|------:|----------|
| Unit breadth | 2 | ~14 logical unit targets; cells/core only |
| Integration design | 4 | Rich matrix (288 queries), thrift C++ client, ranger in-process |
| Integration automation | 3 | Sparse `TEST=2` matrix includes (g++-13 unit-test pairs); not full grid / nightly |
| Golden / wire regression | 1 | Single golden file |
| Sanitizer / coverage | 1 | CMake supports ASan/TSan; unused in CI |
| Docs for testing/CI | 3 | CONTRIBUTING + `docs/build/test/` document `[TEST COMMIT]` gate and `TEST=2` |

### 5.3 Bindings (score 1.5 / 5)

| Binding | Status |
|---------|--------|
| Thrift C++ | Strong integration client (~935 lines) — sparse `TEST=2` CI |
| Thrift C | Minimal smoke (list columns) |
| Python | `add_test` lines **commented out** in `src/py/CMakeLists.txt` |
| Java | Built with `-Dmaven.test.skip=true`; thrift-tests module exists but skipped at package |
| Ruby | Gem only — **no tests** |
| JDBC | Built — **no test module** |
| CI languages | `SWC_LANGUAGES=NONE` |

---

## 6. FS reliability matrix (Phase 2c)

| Backend | Error source | Timeout / retry at backend | Interface sync wrapper |
|---------|--------------|----------------------------|------------------------|
| Local | `errno` / OS | Immediate return | **Insistent** `for(;;)` + 10 ms delay until OK / benign / shutdown |
| Ceph | librados / cephfs | Init: insistent `while (!initialize())`; ops return codes | Same Interface insistence (resume after cluster usable) |
| Hadoop / HadoopJVM | JNI / HDFS API | Backend-specific | Same Interface insistence |
| Broker | RPC + decode (`Base.cc`) | `swc.fs.broker.timeout` (default 120 s) + bytes ratio | Interface insistence **desired**: Broker may time out; Interface retries until FS/Broker recovers and the same op completes |

**Design intent:** Interface-level insistence preserves mid-op I/O state across FS outages. Broker’s finite timeout is a transport bound, not a signal that Interface retries should be capped.

**Durability note:** CommitLog fragment writes rely on FS `write`/`sync` semantics after Interface returns OK. Local durability ≈ fsync behavior of Local backend; Broker durability depends on remote FsBroker completing sync before OK. No automated cross-backend durability matrix exists.

**Async path:** Some Interface async methods re-post the callback on error (process-lifetime insistent), matching the same failure-tolerance model as sync ops.

---

## 7. Prioritized improvement backlog

| Priority | Item | Severity | Effort (est.) | Rationale |
|----------|------|----------|---------------|-----------|
| **1** | Fail-fast on truncated/corrupt CellStore/CommitLog cell reads (set `err`, abort load/split/compact) | P1 | M | Data integrity |
| **2** | Cap CellStore `blks_count` / `idx_size_*` against schema/RAM; fail load on violation | P1 | M | Corrupt metadata DoS/corruption |
| **3** | Widen CI: unit tests without `[TEST COMMIT]` (or on all PRs); nightly/full-matrix `TEST=2`; add broker step | P2 | M | Reduce remaining false confidence |
| **4** | Compaction + CommitLogCompact + Ranger protocol integration tests; enable commented compact path | P2 | L | Cover highest-risk untested code |
| **5** | Null-check `m_metrics` on all `ev->error` paths in AppContexts | P2 | S | Latent crash when metrics off |
| **6** | Log unexpected exceptions in ConnHandler receive/send catches; apply connect timeout | P0/P2 | S | Observability + correctness |
| **7** | Harden `apply_new` (two-phase / fail closed if log remove fails after CS replace) | P1 | M | Compaction atomicity |
| **8** | Extract shared daemon bootstrap; unify manager/ranger to table dispatch | P3 | L | Drift / duplication |
| **9** | Split `CompactRange.cc`, `Range.cc`, `AppHandler.h` | P3 | L | Reviewability |
| **10** | Longer-term: compiled daemon libs instead of `.cc`-in-header | P3 | XL | Build/test modularity |
| **11** | Binding smoke tests + document `[TEST COMMIT]` / sanitizer builds in CONTRIBUTING + docs/build/test | P2 | S–M | Ecosystem drift |

S ≈ days, M ≈ ≤1–2 weeks, L ≈ multi-week, XL ≈ project-scale.

*(Removed former backlog item “Bound FS Interface sync retries”: insistent FS retry is intentional failure-tolerance — see R-P1-5.)*

---

## 8. Suggested ongoing quality gates

1. **Include-graph CI:** fail if `core/` includes outside core/common; fail if daemons cross-include each other.
2. **File-size policy:** warn >600 LOC; fail **new** files >1000 LOC (grandfather existing with tracked exceptions).
3. **Grep gate:** empty `catch(...)` in `core/comm` and `ranger/db` (allowlist intentional Logger cases only).
4. **Track** count of unconditional `#include "*.cc"` in daemon headers — no new ones without review.
5. **ASan unit-test job** (`SWC_ENABLE_SANITIZER=address`) on at least one compiler / `O_LEVEL=3`.
6. **Nightly integration** with `swcdb_cluster` (`TEST=2`), including broker and thrift C++ client.
7. **Protocol drift check:** `MAX_CMD` vs broker/fsbroker handler table size (compile-time assert already natural for tables).

---

## 9. Strengths to preserve

1. Clear product and module topology matching runtime data flow.
2. `core` as a true leaf; no daemon↔daemon header coupling.
3. Centralized `Commands.h` and stable handler/wire separation — good for multi-language bindings.
4. Broker/FsBroker table-driven dispatch — prefer this pattern when touching manager/ranger.
5. CellStore `Readers::replace` rename-with-rollback.
6. **Insistent FS Interface I/O** — sync (and matching async) ops retry until success/benign/shutdown so FS outage → repair/boot leaves in-flight I/O in the same op state (see R-P1-5).
7. Strict `-Wall -Werror` (+ pedantic extras) via `cmake/FlagsWarnings.cmake`.
8. Purpose-built concurrency (`MutexSptd`, IoContext pools, ranger’s three IoContexts).
9. Mature error code taxonomy (`MNGR_`, `RGR_`, `COLUMN_`, `RANGE_`, `FS_`).
10. Serialization primitives that check `remain` before reads (`Serialization.h`, `Cell.h` value length).
11. Local integration design (combinatorial client matrix, thrift coverage) — needs automation, not redesign.

---

## 10. Appendix — Evidence index

| Topic | Primary paths |
|-------|----------------|
| Header overflow | `src/cc/lib/swcdb/core/comm/ConnHandler.cc:303–335`, `ConnHandler.h:246`, `Header.h:20–24,138–169` |
| Payload DoS | `ConnHandler.cc:421–436` |
| Connect timeout | `SerializedClient.cc:33–77` |
| Truncated cells | `CommitLogFragment.cc:442–508`, `RangeBlock.cc:223–258` |
| CellStore index | `CellStore.cc:174–247` |
| Compaction apply | `RangeBlocks.cc:94–108`, `CellStoreReaders.cc:224–258`, `CompactRange.cc:1113–1130` |
| FS insistent retry | `Interface.cc:14–16,183–410`, `fs/Broker/FileSystem.cc:74–77`, `fs/Ceph/FileSystem.cc:86–94` |
| Metrics null | `{manager,ranger,broker,fsbroker}/AppContext.h` `ev->error` branches |
| CI | `.github/workflows/ci.yml` |
| Tests | `tests/libswcdb*`, `tests/integration/**`, `cmake/Utils.cmake` |
| `.cc`-in-header | `ranger/db/Columns.h`, `Compaction.h`, `MngrEnv.h`, `*Env.h` MetricsReporting includes |

---

## 11. Conclusion

SWC-DB’s **architectural intent and build hygiene are above average** for a C++20 distributed DBMS of this age. The evaluation’s critical finding is that **runtime safety on untrusted peers and corrupted on-disk data is weaker than the layering would suggest**, and **CI currently provides false confidence** by compiling widely while testing narrowly.

Next step after accepting this report: implement backlog items **1–2** (comm bounds) and **7** (metrics null-check) as a small high-ROI PR, then **5** (CI gate) and **3–4** / **9** (integrity / compaction apply) as the next wave. Do not bound FS Interface retries — that insistence is intentional failure-tolerance.

---

## 12. Related artifacts

| Artifact | Role |
|----------|------|
| [`.cursor/rules/review-quality.mdc`](../.cursor/rules/review-quality.mdc) | Distilled P0/P1 agent checklist |
| [`STANDARDS_CLARITY_EVALUATION.md`](STANDARDS_CLARITY_EVALUATION.md) | Standards / clarity (Layer A/B) |
| [`DOCS_CLARITY_EVALUATION.md`](DOCS_CLARITY_EVALUATION.md) | User-docs clarity |
| [`AGENTS.md`](../AGENTS.md) | Slim agent index |
| [`CONTRIBUTING.md`](../CONTRIBUTING.md) | Human CI / PR workflow |

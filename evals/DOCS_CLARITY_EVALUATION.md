# SWC-DB User Docs Clarity Evaluation (Readiness / Integrity / Clarity)

**Date:** 2026-07-13  
**Scope:** Published user documentation under [`docs/`](../docs/) (Jekyll site) and how root [`README.md`](../README.md) points at doc mirrors  
**Method:** Section inventory → rubric lock → evidence sampling on high-traffic pages → scored criteria → prioritized backlog  
**Complementary to:** [`STANDARDS_CLARITY_EVALUATION.md`](STANDARDS_CLARITY_EVALUATION.md) (code style/clarity), [`QUALITY_EVALUATION_REPORT.md`](QUALITY_EVALUATION_REPORT.md) (correctness/security/CI). Living docs checklist: [`.cursor/rules/review-docs-clarity.mdc`](../.cursor/rules/review-docs-clarity.mdc).  
**Out of scope:** Code refactors, CI behavior changes, Doxygen body quality, rewriting user guides in this pass (except documenting testing/CI — see backlog notes and `CONTRIBUTING.md`)

---

## 1. Executive summary

| | |
|--|--|
| **Overall (weighted)** | **3.4 / 5.0** (re-scored after 2026-07-13 integrity pass) |
| **Maturity** | Broad operator coverage; factual integrity and journey readiness improved vs prior 2.7 |
| **Top remaining** | (1) Source **0.5.13** vs published packages **0.5.12**, (2) thin CLI admin shells, (3) optional automated link-check CI |

**Recommendation:** Keep **release version sync (P0)** as the only blocking release checklist item. Contributor process stays in [`CONTRIBUTING.md`](../CONTRIBUTING.md); operator guides stay in `docs/`.

---

## 2. Inventory — what the user docs are

### 2.1 Site layout (`docs/`)

| Section | Index | Role |
|---------|-------|------|
| Introduction | [`docs/README.md`](../docs/README.md) | Product overview, checklist, manual TOC |
| Getting Started | [`docs/getting-started/`](../docs/getting-started/) | First-run path: install → run → query |
| Using | [`docs/use/`](../docs/use/) | SQL, CLI, Thrift, C++ client, load generator |
| Running | [`docs/run/`](../docs/run/) | Pseudomode, distributed |
| Configuring | [`docs/configure/`](../docs/configure/) | `.cfg` files, properties, OS tuning, benchmarks |
| Installing | [`docs/install/`](../docs/install/) | Steps, deps, packages, `swcdb_cluster`, thrift clients |
| Building | [`docs/build/`](../docs/build/) | Prerequisites, CMake, make, Doxygen, test |
| Additional docs | [`docs/additional-docs/`](../docs/additional-docs/) | Pointer to https://cpp.swcdb.org |
| Support & license | [`docs/support_license/`](../docs/support_license/) | Support channels, GPLv3 |

**Build/publish:** Jekyll + `StarHPC/jekyll-rtd-theme` ([`docs/_config.yml`](../docs/_config.yml), [`docs/Gemfile`](../docs/Gemfile)); custom domain [`docs/CNAME`](../docs/CNAME) → `swcdb.org`. No docs GitHub Actions workflow; no `.readthedocs.yaml` in-repo.

### 2.2 Doc mirrors (from root README)

| Mirror | Claimed role |
|--------|----------------|
| https://www.swcdb.org | Latest-release docs + additional-docs origin |
| https://kashirin-alex.github.io/swc-db/ | Current `master` docs |
| https://swc-db.readthedocs.io/ | ReadTheDocs (linked; no in-repo RTD config found) |

Users are not told which mirror to prefer when versions diverge.

### 2.3 Outside `docs/` (not scored as user-docs content)

| Artifact | Role |
|----------|------|
| [`CONTRIBUTING.md`](../CONTRIBUTING.md) | Legal + commits / CI / PR rules |
| [`AGENTS.md`](../AGENTS.md) | Slim agent index |
| [`.cursor/rules/`](../.cursor/rules/) | Agent conventions + review checklists |
| [`evals/`](.) | Dated scored audits (this directory) |

---

## 3. Rubric (locked)

Scores are 1–5. Weight sums to 100%.

| Criterion | Weight | What “5” looks like |
|-----------|-------:|---------------------|
| Journey readiness | 20% | Clear Getting Started; hubs orient; install→run→use without dead ends |
| Factual integrity | 25% | Versions, deps, defaults, and examples match the current release / build |
| Link / nav integrity | 20% | No broken paths; TOC and sidebar agree on important pages |
| Structural clarity | 15% | Section purpose obvious; mirrors explained; consistent terminology |
| Task completeness | 10% | Task pages (test, generate docs, first cluster) are actionable end-to-end |
| Language clarity | 10% | Readable English on high-traffic pages; typos do not impede meaning |

---

## 4. Scores and evidence

### 4.1 Scorecard

| Criterion | Score | Weighted |
|-----------|------:|---------:|
| Journey readiness | 3 | 0.60 |
| Factual integrity | 4 | 1.00 |
| Link / nav integrity | 3 | 0.60 |
| Structural clarity | 4 | 0.60 |
| Task completeness | 3 | 0.30 |
| Language clarity | 3 | 0.30 |
| **Overall** | | **3.40** |

### 4.2 Journey readiness — **2 / 5** (improved; re-score after user feedback)

**Evidence**

- [`docs/getting-started/README.md`](../docs/getting-started/README.md) added with install → configure → pseudomode → first-query path; linked from intro and install steps.
- Section hubs now list child pages with one-line purpose (no longer stub-only).
- [`docs/install/steps/README.md`](../docs/install/steps/README.md) lists ordered steps and links to build-from-source and Getting Started.

**Remaining:** Pseudomode still assumes edited cfg without a minimal template; C++ client guide thinner than Thrift/CLI.

### 4.3 Factual integrity — **4 / 5**

**Evidence (after 2026-07-13 integrity pass)**

| Issue | Status |
|-------|--------|
| SQL compact example, broker/ranger encoder defaults, Thrift `Comp` OR, `SWC_BUILD_PKG` typo/missing `broker` | **Fixed** |
| CMake `SWC_LANGUAGES` / undocumented flags / O_LEVEL | **Aligned** |
| Performance claim cites **v0.4.9** while product is **0.5.13** | Labeled historical (prior pass) |
| Source tree **0.5.13** vs downloadable packages **0.5.12** | Documented gap; bump on next package release |

**Strength:** Property reference, SQL/Thrift catalogs, and install/build guides track the codebase for day-to-day operator tasks.

### 4.4 Link / nav integrity — **2 / 5**

**Evidence**

- Broken relative links to `netstd/` and `rust/` under `docs/use/thriftclient/` — **fixed in this pass** (plain-text “generated-only” note, no dead links).

- Intro TOC omitted pages that exist in the tree / sidebar: `use/client_library/`, `install/thrift_clients/`, `configure/os_tuning/` (canonical; `os_tunning/` is a `nav_exclude` redirect), `configure/benchmarks/`, `additional-docs/`, `getting-started/` — **synced in this pass**.

- `_config.yml` and intro referenced `logo.svg` but only `logo-big.svg` existed — **fixed in this pass**.

- [`docs/install/getting_swcdb/README.md`](../docs/install/getting_swcdb/README.md) linked `#download` but heading is `#available-for-download` — **fixed in this pass**.
- [`docs/use/client_library/README.md`](../docs/use/client_library/README.md) is a full C++ client guide but was absent from the manual TOC in [`docs/README.md`](../docs/README.md) — **added in this pass**.

**Strength:** Primary section links in the intro TOC (SQL, Thrift, CLI, run modes, build steps) resolve to real pages.

### 4.5 Structural clarity — **3 / 5**

**Evidence**

- Sensible top-level IA: Using / Running / Configuring / Installing / Building / Support.
- Consistent Jekyll frontmatter (`title`, `sort`) and `jekyll-readme-index` hubs.
- Canonical path is [`docs/configure/os_tuning/`](../docs/configure/os_tuning/); misspelled [`docs/configure/os_tunning/`](../docs/configure/os_tunning/) remains as a redirect with `nav_exclude: true` (sidebar filter in `_includes/common/rest/site_pages.liquid`). Remaining polish: prefer “Documentation” over “Documentations” in section titles over time.
- Three mirrors listed in root README — **roles clarified in this pass** (www = latest release; GitHub Pages = master; RTD = additional mirror).

**Strength:** Architecture concepts and system columns in the introduction give useful mental model before ops pages.

### 4.6 Task completeness — **2 / 5**

**Evidence**

- [`docs/build/test/README.md`](../docs/build/test/README.md) was only `make test` + `swcdb_cluster` pointer; no unit vs integration distinction, no GitHub Actions / `[TEST COMMIT]` (addressed in this pass for humans via CONTRIBUTING + expanded test page).
- [`docs/build/documentations/README.md`](../docs/build/documentations/README.md) covers `make doc` briefly; no publish path for cpp.swcdb.org.
- Contributor workflow was absent from human docs (legal-only `CONTRIBUTING.md`) — **gap closed in this pass** for commits/PRs/CI; still no Jekyll “contributing” chapter (intentional).

**Strength:** Pseudomode and distributed run guides, property reference tree, and Thrift language guides support real operator/integrator tasks when versions match.

### 4.7 Language clarity — **3 / 5**

**Sample issues (high-traffic / visible)**

| Typo / wording | Path | Status |
|----------------|------|--------|
| “Structual”, “facillity”, “Tripel Stores” | `docs/README.md` | **Fixed in this pass** |
| “STARING-UP”, “commdand” | `docs/run/pseudomode/` / `distributed/` | **Fixed in this pass** |
| “instructiuons” | `docs/build/test/README.md` | **Fixed in this pass** |
| “Documetaions” | `docs/additional-docs/cpp.md` | **Fixed in this pass** |
| “Commiter” in API link labels | `docs/use/client_library/README.md` | **Fixed in this pass** |
| “thiriftbroker” in build options table | `docs/build/configure/README.md` | **Fixed in this pass** |

High-traffic typos from the sample set above were corrected in the integrity pass; residual English polish may remain elsewhere.

---

## 5. Strengths to preserve

1. Deep property reference under `docs/configure/properties/` (per-daemon and FS backends).  
2. SQL + Thrift service reference material in one place for application developers.  
3. Multi-distro build prerequisites (`debian_ubuntu`, `archlinux`, LFS).  
4. Explicit install packaging paths (tar.xz, deb, install-pack, AUR).  
5. Architecture / cell-key / system-column explanation on the intro page.  
6. Separation of operator docs (`docs/`) from engineering evals (`evals/`) and agent rules (`.cursor/rules/`).

---

## 6. Prioritized improvement backlog

| Priority | Item | Severity | Effort | Rationale |
|----------|------|----------|--------|-----------|
| **1** | On release, add 0.5.13+ rows to download table and bump `swc.install.archive` + examples together | P0 | S | Install integrity — examples use **0.5.12** (latest published) |
| **2** | ~~Fix or remove broken `netstd/` and `rust/` Thrift links~~ | P0 | S | **Done** |
| **3** | ~~Refresh intro performance note (date-stamp as historical)~~ | P0 | S | **Done** |
| **4** | ~~Align dependency Thrift version docs with CI/build (0.20.x) and Java classpath~~ | P0 | S–M | **Done** |
| **5** | ~~Add Getting Started + flesh section hubs with install→pseudomode→SQL/CLI path~~ | P1 | M | **Done** |
| **6** | ~~Sync intro TOC with sidebar~~ | P1 | S | **Done** |
| **7** | ~~Expand `build/documentations` with what `make doc` emits and where published API docs live~~ | P1 | S | **Done** |
| **8** | ~~Document mirror roles (www vs GitHub Pages vs RTD) in root README~~ | P1 | S | **Done** |
| **9** | ~~Editorial pass + `os_tunning` → `os_tuning` with redirect stub~~ | P2 | M | **Done** (`nav_exclude` on stub so sidebar shows one entry) |
| **10** | ~~Jekyll preview steps for doc contributors~~ | P2 | S | **Done** (in CONTRIBUTING.md) |
| **11** | Thin CLI admin shells (`manager`/`ranger`/`filesystem`) beyond help dumps | P2 | M | Optional polish |
| **12** | Optional CI job for relative link check under `docs/` | P2 | M | Anti-drift |

S ≈ hours–day, M ≈ ≤1 week.

**Done in the 2026-07-13 integrity pass:** P0 copy-paste fixes (SQL compact, broker/ranger encoder defaults, Thrift `Comp` OR, `SWC_BUILD_PKG` broker/thriftbroker, O_LEVEL); CMake/languages/flags alignment; SQL dump/load + modify aliases; install deps→build prereqs; thrift_clients (Java/JDBC/C++/C-Glib); journey pages (getting-started, pseudomode, swcdb_cluster, c_glib, make doc, benchmarks image); CONTRIBUTING docs gates + PR template; `os_tuning` rename with `nav_exclude` redirect; sidebar filter honors `nav_exclude`.

---

## 7. Suggested ongoing docs gates

1. **Version check:** on release, update `getting_swcdb` table, `swc.install.archive` default, and any `SWCDB_VERSION=` examples together.  
2. **Link check:** fail doc PRs on broken relative links under `docs/` (especially `thriftclient/*`).  
3. **TOC sync:** if a new `docs/**/README.md` with `title:` is added, require a line in the intro TOC, an explicit “sidebar-only” note, or `nav_exclude: true` for redirect stubs.  
4. **Hub non-stubs:** section indexes should list child pages with one-line purpose (not only “Navigate the side-menu”).  
5. **Code↔docs sync:** changes to comparators / Thrift `Comp`, CMake options, or `src/etc/swcdb` defaults update matching docs in the same PR (also listed in [`CONTRIBUTING.md`](../CONTRIBUTING.md)).

---

## 8. Related artifacts

| Artifact | Role |
|----------|------|
| [`CONTRIBUTING.md`](../CONTRIBUTING.md) | Legal + commits / CI / PR / docs-PR rules |
| [`.github/pull_request_template.md`](../.github/pull_request_template.md) | PR description checklist |
| [`docs/build/test/README.md`](../docs/build/test/README.md) | Local test + CI gate notes |
| [`QUALITY_EVALUATION_REPORT.md`](QUALITY_EVALUATION_REPORT.md) | CI truth table; scored “Docs for testing/CI” |
| [`STANDARDS_CLARITY_EVALUATION.md`](STANDARDS_CLARITY_EVALUATION.md) | Code Layer A/B checklist for PRs |
| [`.cursor/rules/review-docs-clarity.mdc`](../.cursor/rules/review-docs-clarity.mdc) | Agent docs-PR checklist |
| [`AGENTS.md`](../AGENTS.md) | Slim agent index |
| [`docs/_config.yml`](../docs/_config.yml) | Jekyll / theme / site URL |

---

## 9. Next step after this pass

On the next **release**, add package rows for the new version and bump `swc.install.archive` + all `SWCDB_VERSION=` examples together (backlog **1**). Remaining optional polish: thin CLI admin shells (**11**), automated link-check CI (**12**). Keep contributor process in `CONTRIBUTING.md`; use §7 gates when reviewing documentation PRs.

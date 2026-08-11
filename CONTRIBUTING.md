
# Contributing to SWC-DB



There are many ways/forms to contribute to the SWC-DB project and You are welcome to make your contributions; Whereas before you proceed with contributing you need to understand and agree on this(CONTRIBUTING.md) file conditions and definitions By making a contribution you consent and agree to the content of [this(CONTRIBUTING.md) file](https://github.com/kashirin-alex/swc-db/blob/master/CONTRIBUTING.md).



### The Types(classification) of Contributions
The Contribution Types consist of Issues(support), Pull-Request(development), Publications(information) and Forum/Groups/Discussion Topics(support & information) which were created in relation on SWC-DB project correspond to the source-code and as a software described at [www.swcdb.org](https://www.swcdb.org) and [github.com/kashirin-alex/swc-db](https://github.com/kashirin-alex/swc-db/)



### The Permission you Grant to the SWC-DB project by Contributing
You grant SWC-DB project the rights(copyrights without the requirement of crediting the author) to use the provided-information by either types of the listed Contributions. SWC-DB project at its sole-discretion will decide whether the rights of the provided-information are subject to 3rd(other) parties involved and act-accordingly by limiting the Permission Grant on the Information received. The "Contributor(you) Grant Rights" to the full-meaning of expression is that SWC-DB project is allowed to use the provided-information in the SWC-DB proprietary licenses and the copyrights are unclaimable by the Contributor.



### The License that is applied to the Contributions
Whereas you(Contributor) Grant Rights to and under `SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>` copyright holder.
The Contributions which were made in the public domain (under this file conditions and definitions). The contribution-content, included in the source-code and consist in releases of SWC-DB, in the public domain will be available under the GPLv3 license as defined at [The License](https://github.com/kashirin-alex/swc-db/#license).



### An Extended Requirement for Development Contributions
The Requirement of a Development Contribution, that consists of adding Code to the source of SWC-DB project, requires that the author owns all the copyrights of the contribution content. If a new file is added, the header of the file has to state in comments(format depends on the programming-language):
```CPP
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 ```



### The Contributors Credits File
For the purpose of crediting the Contributor. There is the CREDITS.md file and for past years the CREDITS_YYYY.md files.
The Contributor can be asked and can ask to be added-to or removed-from the credits file.
The required format:
```
Author Name/Nickname<mail@> (YYYY/MM/DD):
  The Description of Contribution.
  References to Pull-Requests/Issues (SWC-DB project urls) are allowed.

Author ..
```



### Development Workflow
For development (Pull-Request) contributions:

1. Fork or branch from `master`.
2. Make focused changes; match neighboring code style (there is no `.clang-format` / `.editorconfig`).
3. Build and test locally (see [Local Testing](#local-testing) and [docs/build/test/](docs/build/test/)).
4. Open a Pull Request against `master` (or `CI-test` when intentionally exercising CI).

Compiler warnings are errors (`cmake/FlagsWarnings.cmake`). Prefer small, reviewable PRs.

**Code style:** There is no `.clang-format`. Match neighboring code; use the Layer A / Layer B checklist in [`evals/STANDARDS_CLARITY_EVALUATION.md`](evals/STANDARDS_CLARITY_EVALUATION.md) §9 (also `.cursor/rules/review-standards-clarity.mdc`, `cpp-conventions.mdc`, Thrift layer rules).



### Commit Messages
Write a clear subject that states why the change exists. A short body is welcome when the rationale is not obvious from the diff.

#### Triggering GitHub Actions CI — `[TEST COMMIT]`
CI (`.github/workflows/ci.yml`) runs **only** when the **head** commit message contains the exact substring `[TEST COMMIT]`.

- Include `[TEST COMMIT]` when you intentionally want the workflow to run on push or pull request.
- The workflow uses a small `gate` job that checks out the push tip or PR head SHA and inspects that commit’s message (pull_request events do not set `github.event.head_commit`).
- If the marker is absent from the head commit, **do not assume CI ran** — the main job is skipped.
- Example subject: `Fix range unload path [TEST COMMIT]`

There is no required Conventional Commits format; keep messages readable and accurate.



### What CI Runs Today
When triggered with `[TEST COMMIT]`, the workflow builds a matrix on Ubuntu with several compilers, `O_LEVEL` values, and `SWC_IMPL_SOURCE` ON/OFF. Default configure uses `-DSWC_LANGUAGES=NONE` and install prefix `/opt/swcdb`. Default Thrift version in CI is `0.20.0`; a sparse matrix `include` also builds `0.23.0`.

| Fact | Behavior |
|------|----------|
| Opt-in gate | `gate` job opens only if the head commit message contains `[TEST COMMIT]` (push tip or PR head) |
| Matrix `TEST` | Default is `1`; sparse `include` adds `TEST=2` on g++-11 with the unit-test O_LEVEL/IMPL pairs |
| Unit tests | Run only on a subset of the matrix (`O_LEVEL=3`+`IMPL=OFF` or `O_LEVEL=6`+`IMPL=ON`) |
| Integration | Steps gated on `TEST == '2'` — run on the sparse `TEST=2` includes above |
| Thrift | Default `0.20.0`; sparse `include` compiles `thriftgen-0.23.0` (`THRIFT=0.23.0`, `TEST=1`) |
| Languages / bindings | Not built in default CI (`SWC_LANGUAGES=NONE`) |
| Sanitizers | Supported by CMake locally; not configured as CI jobs |

Local `make test` and a full cluster still matter for confidence that CI does not cover. See also [`evals/QUALITY_EVALUATION_REPORT.md`](evals/QUALITY_EVALUATION_REPORT.md) for a detailed CI truth table.



### Pull Requests
Use the repository Pull Request template. In the description:

- Summarize the change and why it is needed.
- List how you tested (local build/test, and whether you used `[TEST COMMIT]`).
- For **new** source files, include the copyright header above.
- For code style and Thrift layer boundaries, follow the checklist in [`evals/STANDARDS_CLARITY_EVALUATION.md`](evals/STANDARDS_CLARITY_EVALUATION.md) §9 (Layer A hand-written vs Layer B generated). A short agent-oriented copy lives in `.cursor/rules/review-standards-clarity.mdc`.

Do not hand-edit Thrift `thriftgen-*` / `gen-*` trees for behavior or style; change the IDL and/or hand-written wrappers, then regenerate.



### Local Testing
- Prefer a [CI-like smoke configure](docs/build/configure/) (`-DSWC_LANGUAGES=NONE`, `-DBUILD_LINKING=SHARED`, `-DCMAKE_INSTALL_PREFIX=/opt/swcdb`, leave `SWC_BUILD_PKG` empty) before a full language build.
- Unit targets first: `ctest -R libswcdb_core --output-on-failure` (or `libswcdb`) from the build directory — see [Testing](docs/build/test/).
- Full / integration `make test` expects an installed tree and `swcdb_cluster` ([setup](docs/install/swcdb_cluster/)). Before retest: `swcdb_cluster stop` (then `kill` if needed).
- Host tools: `diffutils` for golden `testdiff`; UTF-8 locale for PyPy.
- Optional: sanitizer builds via CMake (`SWC_ENABLE_SANITIZER` = `address` or `thread`) when debugging memory/concurrency issues.

Document what you ran in the PR.



### Dual build model (libraries vs daemons)
Reusable libraries (`core`, `db`, `fs`) compile from `lib/**/*.cc`; headers may also pull `.cc` when `SWC_IMPL_SOURCE` is ON. Daemon logic (manager / ranger / broker / fsbroker) aggregates `.cc` into Env headers unconditionally — there is no separate daemon shared library unless packaging requires it. Agent detail: [`.cursor/rules/build-impl-source.mdc`](.cursor/rules/build-impl-source.mdc).



### Documentation Pull Requests
User-facing docs live under `docs/` (Jekyll). Contributor process for commits/PRs stays in this file (not duplicated as a full chapter on the docs site).

- Preserve YAML frontmatter (`title`, `sort`) on section pages.
- Prefer fixing factual integrity (versions, broken links, TOC gaps) over drive-by rewrites.
- Preview locally with the Gemfile under `docs/` (`bundle install`, then `bundle exec jekyll serve` from `docs/`).
- Scope and backlog for docs clarity: [`evals/DOCS_CLARITY_EVALUATION.md`](evals/DOCS_CLARITY_EVALUATION.md) (agent checklist: `.cursor/rules/review-docs-clarity.mdc`).
- Redirect-only stubs (keep old URLs alive) should set `nav_exclude: true` so they do not duplicate the sidebar entry for the canonical page.

**Integrity gates for docs PRs:**

1. **Release sync:** When publishing a new version, update in one change: `docs/install/getting_swcdb/` download table, `swc.install.archive` default in `docs/configure/properties/swcdb_cluster.md`, and any `SWCDB_VERSION=` examples.
2. **Link check:** Verify new/changed relative links under `docs/` (especially `use/thriftclient/*`).
3. **TOC sync:** If you add a new `docs/**/README.md` with `title:`, add a line to the intro TOC in `docs/README.md`, mark it sidebar-only in the PR, or set `nav_exclude: true` for redirect stubs.
4. **Code↔docs sync:** Changes to `Comparators.h` / Thrift `Comp`, CMake options in `cmake/` / `docs/build/configure/`, or defaults in `src/etc/swcdb/*.dyn.cfg` should update the matching docs in the same PR.



---



>### You need to review this file before your contributions as the file is subject to changes!



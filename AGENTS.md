# SWC-DB — Agent index

Agent entry point for the Super Wide Column Database (SWC-DB) monorepo.
C++20 distributed DBMS; version `0.5.13`; GPLv3.

## Where to look

| Need | Location |
|------|----------|
| Module / daemon / namespace / glossary map | [`.cursor/rules/architecture-overview.mdc`](.cursor/rules/architecture-overview.mdc) (`alwaysApply`) |
| Path-scoped conventions (C++, Thrift, build, daemons, tests) | [`.cursor/rules/`](.cursor/rules/) — loaded by globs |
| Review checklists (standards, quality, docs, CI) | [`.cursor/rules/review-*.mdc`](.cursor/rules/) — requestable / docs-globbed |
| Human commits, `[TEST COMMIT]`, PRs, local testing | [`CONTRIBUTING.md`](CONTRIBUTING.md) |
| Smoke build / unit vs integration / cluster hygiene | [`docs/build/test/`](docs/build/test/), [`.cursor/rules/build-testing.mdc`](.cursor/rules/build-testing.mdc) |
| Major development phases | [`ROADMAP.md`](ROADMAP.md) |
| Dated scored audits | [`evals/`](evals/) |

## Layers at a glance

| Layer | Paths | Policy |
|-------|-------|--------|
| **A Hand-written** | `src/cc/**` (except `gen-*`), Thrift IDL/CMake, `thrift/{broker,client,utils}/` | Naming, copyright, errors/logging, placement — see `cpp-*`, `thrift-handwritten-layer` rules |
| **B Generated** | `thriftgen-*`, `gen-cpp`, `gen-c_glib`, other `gen-*` | Do not edit; regenerate from IDL — see `thrift-generated-layer` |

PR checklist: [`.cursor/rules/review-standards-clarity.mdc`](.cursor/rules/review-standards-clarity.mdc). Full scored reports: [`evals/`](evals/).

## Rule naming

| Prefix | Concern |
|--------|---------|
| `architecture-` | Always-on product map |
| `cpp-` | Hand-written C++ |
| `thrift-` | Thrift IDL / wrappers / generated |
| `daemon-` | Daemon bootstrap and wire commands |
| `build-` | CMake / dual build model / tests |
| `review-` | On-demand or docs-scoped checklists |

Place new code under the matching `src/cc/.../swcdb/<module>/` tree; do not invent parallel module layouts.

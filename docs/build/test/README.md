---
title: Test
sort: 5
---



# Testing SWC-DB build



## Test Preparations

Leave **`SWC_BUILD_PKG` empty** (full tree) so CMake registers tests and examples. Prefer the [CI-like smoke configure]({{ site.baseurl }}/build/configure/#ci-like-smoke--unit-test-build-c-only) for a first local build.

**Host tools and environment**
- `diff` (`diffutils`) — required by golden `tests/testutils/testdiff` (e.g. `properties_parser`)
- UTF-8 locale for PyPy binding steps (`C.utf8`, or `locale-gen` for `en_US.UTF-8`)

**Integration / full `make test`**
- Build and [install]({{ site.baseurl }}/build/make/) first
- Set up [`swcdb_cluster`]({{ site.baseurl }}/install/swcdb_cluster/) (Fabric + passwordless SSH)



## Unit vs integration

| Kind | Paths | Needs install + cluster? | CI coverage |
|------|-------|--------------------------|-------------|
| Unit (`libswcdb_core`) | `tests/libswcdb_core/` | No | Yes on subset (`TEST=1` or `2`, specific O_LEVEL/IMPL) |
| Unit (`libswcdb`) | `tests/libswcdb/` | No | Yes on subset (`TEST=1` or `2`, specific O_LEVEL/IMPL) |
| Integration | `tests/integration/{client,comm,fs,manager,ranger,broker,thrift,utils}/` | Yes | Sparse `TEST=2` matrix includes (not the full compiler grid) |

Prefer unit targets for local C++/core changes. Extend or run integration only when cluster behavior, install layout, or a daemon path is involved.



## Running tests

* while at builds path [as by instructions]({{ site.baseurl }}/build/prerequisites/)

```bash
cd builds/swcdb;   # or your cmake build dir
```

**One module / pattern (preferred while developing):**

```bash
ctest -R libswcdb_core --output-on-failure
# examples: -R libswcdb , -R ranger , -R thrift
```

**Full suite:**

```bash
make test;
```

What runs depends on configure options and whether a cluster is available.



## Cluster hygiene before retest

Leftover daemons hold ports and make `swcdb_cluster start` / `wait_ready` hang or look “flaky”. Before another integration run:

```bash
cd /opt/swcdb   # or your CMAKE_INSTALL_PREFIX
sbin/swcdb_cluster stop
sbin/swcdb_cluster kill   # if stop left processes behind
```

Wipe `/var/opt/swcdb` (or your data dir) only when you intentionally want a clean store — not for every retest.

If `start` reports success but ThriftBroker is down, check [Thrift shared libraries at runtime]({{ site.baseurl }}/install/dependencies/#thrift-shared-libraries-at-runtime) before assuming a test bug.



## GitHub Actions CI and `[TEST COMMIT]`

Automated CI (`.github/workflows/ci.yml`) does **not** run on every push or pull request. A `gate` job opens the main matrix only if the **head** commit message (push tip or PR head SHA) contains `[TEST COMMIT]`.

| Note | Detail |
|------|--------|
| Opt-in | Without `[TEST COMMIT]` on the head commit, assume the main CI job did not run |
| Default matrix | Builds several compilers / `O_LEVEL` / `SWC_IMPL_SOURCE` with `THRIFT=0.20.0` and `TEST=1`; unit tests only on a subset |
| Integration in CI | Sparse matrix `include` entries set `TEST=2` (g++-11, unit-test O_LEVEL/IMPL pairs) so integration steps run |
| Thrift 0.23.0 | Sparse `include` builds with `THRIFT=0.23.0` so `thriftgen-0.23.0` is compiled in CI |
| Languages | Default CI configure uses `-DSWC_LANGUAGES=NONE` |

Contributor rules for commits, pull requests, and when to trigger CI: [CONTRIBUTING.md](https://github.com/kashirin-alex/swc-db/blob/master/CONTRIBUTING.md).



## Optional sanitizer builds

For local debugging, CMake supports `SWC_ENABLE_SANITIZER` set to `address` or `thread`. Sanitizer jobs are not part of the default GitHub Actions matrix.

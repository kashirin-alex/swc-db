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

| Kind | Paths | Needs install + cluster? | Default CI (`TEST=1`) |
|------|-------|--------------------------|------------------------|
| Unit (`libswcdb_core`) | `tests/libswcdb_core/` | No | Yes (subset of matrix) |
| Unit (`libswcdb`) | `tests/libswcdb/` | No | Yes (subset of matrix) |
| Integration | `tests/integration/{client,comm,fs,manager,ranger,broker,thrift,utils}/` | Yes | No — steps need `TEST=2` |

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

Automated CI (`.github/workflows/ci.yml`) does **not** run on every push or pull request. The workflow job starts only if the commit message contains `[TEST COMMIT]`.

| Note | Detail |
|------|--------|
| Opt-in | Without `[TEST COMMIT]`, assume CI did not run |
| Default matrix | Builds several compilers / `O_LEVEL` / `SWC_IMPL_SOURCE`; unit tests only on a subset |
| Integration in CI | Default matrix `TEST` is `1`; steps that require `TEST=2` are not in the default matrix |
| Languages | Default CI configure uses `-DSWC_LANGUAGES=NONE` |

Contributor rules for commits, pull requests, and when to trigger CI: [CONTRIBUTING.md](https://github.com/kashirin-alex/swc-db/blob/master/CONTRIBUTING.md).



## Optional sanitizer builds

For local debugging, CMake supports `SWC_ENABLE_SANITIZER` set to `address` or `thread`. Sanitizer jobs are not part of the default GitHub Actions matrix.

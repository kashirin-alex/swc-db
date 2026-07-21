---
title: Test
sort: 5
---



# Testing SWC-DB build



## Test Preparations

**The test uses the swcdb_cluster**
- swcdb_cluster is required to be set [as by instructions]({{ site.baseurl }}/install/swcdb_cluster/)

Integration-style coverage via `make test` expects an installed SWC-DB tree and a working cluster setup. Build and install first using the [Build]({{ site.baseurl }}/build/) and [Install]({{ site.baseurl }}/install/) guides.



## Running the SWC-DB build Test
*  while at builds path [as by instructions]({{ site.baseurl }}/build/prerequisites/)

```bash
cd builds/swcdb;   # or your cmake build dir
```

```bash
make test;
```

What runs depends on how the tree was configured and whether a cluster is available. Prefer verifying the specific area you changed (core/db unit targets, ranger/thrift integration, etc.) before relying on a full `make test`.



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

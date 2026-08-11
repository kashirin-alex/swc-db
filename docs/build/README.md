---
title: Building
sort: 5 
---

# Building SWC-DB

Build and test SWC-DB from source.

| Guide | Purpose |
|-------|---------|
| [Build Steps]({{ site.baseurl }}/build/steps/) | Ordered build flow |
| [Prerequisites]({{ site.baseurl }}/build/prerequisites/) | Toolchain, source checkout, 3rd-party deps |
| [Configure]({{ site.baseurl }}/build/configure/) | CMake options (`SWC_BUILD_PKG`, `SWC_LANGUAGES`, etc.) |
| [Make]({{ site.baseurl }}/build/make/) | Compile and install |
| [Documentations]({{ site.baseurl }}/build/documentations/) | Generate Doxygen API docs (`make doc`) |
| [Test]({{ site.baseurl }}/build/test/) | Unit and integration tests, CI notes |

To install pre-built packages instead, see [Installing]({{ site.baseurl }}/install/).

## Dual build model (short)

- **Libraries** (`core`, `db`, `fs`): compiled from `lib/**/*.cc`. With `-DSWC_IMPL_SOURCE=ON`, headers may also include their `.cc` implementation.
- **Daemons** (manager, ranger, broker, fsbroker): Env headers aggregate `.cc` files unconditionally (no separate daemon shared library unless packaging needs one).

CMake helpers and `SWC_BUILD_PKG` gates are described for contributors in the repository rule [`.cursor/rules/build-impl-source.mdc`](https://github.com/kashirin-alex/swc-db/blob/master/.cursor/rules/build-impl-source.mdc) and [CONTRIBUTING.md](https://github.com/kashirin-alex/swc-db/blob/master/CONTRIBUTING.md).

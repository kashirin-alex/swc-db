---
title: Dependencies
sort: 2
---

# Dependencies Installation

## Thrift shared libraries at runtime

Apache Thrift **0.13.0+** is required for Thrift clients and broker; CI validates with **0.20.0**. Distro packages may ship an older version — build Thrift from source if code generation or linking fails.

Installed SWC-DB binaries use RPATH `${CMAKE_INSTALL_PREFIX}/lib` (for example `/opt/swcdb/lib`). If `libthrift*.so*` lives elsewhere (common when Thrift was built into `/usr/local/lib`), `swcdbThriftBroker` can fail with exit 127 / “library not found” while Manager/Ranger still start. Fixes (pick one):

* Configure with `-DSWC_INSTALL_DEP_LIBS=ON` so CMake installs linked dependency shared libs into the prefix `lib/` on `make install`
* Copy or symlink `libthrift*.so*` into `${CMAKE_INSTALL_PREFIX}/lib`
* Add the Thrift lib directory to the dynamic linker config (`ldconfig`) or set `LD_LIBRARY_PATH` for cluster start



## on Debian/Ubuntu

All 3rd-part dependencies are available from the os-distribution.

_Update your repositories if outdated._
```bash
apt-get update;
```

_Libraries Versions and Availability might vary between Ubuntu Releases. Package names below are for Ubuntu 22.04+; on older releases use `libtcmalloc-minimal5` and/or `libre2-5` if the packages below are not found._

```bash
apt-get install -y \
  libtcmalloc-minimal4 \
  libre2-9 \
  libsnappy1v5 \
  libthrift-dev \
  libreadline8 \
  libssl-dev \
  python3-pip ;
```


## on Archlinux

All 3rd-part dependencies are available from the os-distribution.

_Update your repositories if outdated._
```bash
pacman -Syu;
```

_Install the Libraries_
```bash
pacman -S \
  snappy \
  re2 \
  gperftools \
  boost \
  libevent \
  thrift;
```


> **Building from source?** These packages are runtime-oriented. For compilers, CMake, ASIO, and other build dependencies, follow [Build Prerequisites]({{ site.baseurl }}/build/prerequisites/) (including the [Debian/Ubuntu]({{ site.baseurl }}/build/prerequisites/environment/debian_ubuntu/) or [Archlinux]({{ site.baseurl }}/build/prerequisites/environment/archlinux/) environment pages).



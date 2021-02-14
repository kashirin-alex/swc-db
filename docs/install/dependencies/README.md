---
title: Dependencies
sort: 2
---

# Dependencies Installation



## on Debian/Ubuntu

All 3rd-part dependencies are available from the os-distribution.

_Update your repositories if outdated._
```bash
apt-get update;
```

_Libraries Versions and Availability might vary between Ubuntu Releases._
```bash
apt-get install -y \
  libtcmalloc-minimal(4|5) \
  libre2-(4|5) \
  libsnappy1v5 \
  libthrift-0.13.0 \
  libreadline8 \
  libssl \
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





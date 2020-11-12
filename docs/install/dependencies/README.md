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
  python3-pip ;
```



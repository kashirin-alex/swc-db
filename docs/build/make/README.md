---
title: Making
sort: 4
---


# Making

*  while at builds [path as by instructions](/build/prerequisites/)

```
cd swcdb; 
```


## Building
_Parallel build is supported_

```
make -j8;
```


## Installing
```
make install;
```


## Packaging
_SWCDB_BUILD_VERSION suggested format [**#.#.#.RelaseType.ARCH**]_

### .tar.xz
```
cd /opt/swcdb/;
XZ_OPT=-e9 tar -cJf ../swcdb-{SWCDB_BUILD_VERSION}.tar.xz .;
```

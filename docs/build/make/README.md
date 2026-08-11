---
title: Making
sort: 4
---


# Making

*  while at builds [path as by instructions]({{ site.baseurl }}/build/prerequisites/)

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

Installed executables embed RPATH `${CMAKE_INSTALL_PREFIX}/lib`. If Apache Thrift (or other deps) were built outside that prefix, see [Dependencies — Thrift shared libraries at runtime]({{ site.baseurl }}/install/dependencies/#thrift-shared-libraries-at-runtime) before starting `swcdbThriftBroker`.


## Packaging
_SWCDB_BUILD_VERSION suggested format [**#.#.#.RelaseType.ARCH**]_

### .tar.xz
```
cd /opt/swcdb/;
XZ_OPT=-e9 tar -cJf ../swcdb-{SWCDB_BUILD_VERSION}.tar.xz .;
```

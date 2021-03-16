---
title: Configure
sort: 3
---



# Configuring SWC-DB build

## SWC-DB Configuration Options

| CONFIG OPTION | DESCRIPTION | VALUE OPTIONS | DEFAULT VALUE |
| ---  | --- | --- | --- |
|O_LEVEL| Level of optimizations: <br/>  0: = -Os <br/>  1: = -O2s <br/>  2: += -floop-interchange -flto -fuse-linker-plugin -ffat-lto-objects <br/>  3: = -O3 <br/>  4: += -flto -fuse-linker-plugin -ffat-lto-objects<br/>  5: += BUILD_LINKING=STATIC <br/>  6: += BUILD_LINKING_CORE=STATIC | 0-7 | 3 |
|SWC_IMPL_SOURCE| when possible implement SWC-DB source-code | ON/OFF | OFF |
|USE_GNU_READLINE| whether to use GNU libreadline | ON/OFF | OFF(ON if EDITLINE not found) |
|LOOKUP_INCLUDE_PATHS| additional paths to headers | posix-dir-path_LIST; | "/opt/local/include;/usr/local/include;usr/local/lib;/usr/include" |
|LOOKUP_LIB_PATHS| additional paths to libraries | posix-dir-path_LIST; | "/opt/local/lib;/usr/local/lib;/usr/lib;/lib" |
|JAVA_INSTALL_PATH| JAVA_HOME to use, suggested ```$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g)``` | posix-dir-path | ENV{JAVA_HOME} |
|ASIO_INCLUDE_PATH| suggested [as by instructions]({{ site.baseurl }}/build/prerequisites/specific/#version-asio) | posix-dir-path | "" |
|WITHOUT_THRIFT_C| Not to build the libswcdb_thrift_c | ON/OFF | OFF |
|GLIB_INCLUDE_PATH| suggested ```$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )``` | posix-dir-path | "" |
|WITHOUT_PAM| Not to build the libpam_swcdb_max_retries | ON/OFF | OFF |
|HADOOP_INSTALL_PATH| HADOOP_HOME to use, suggested [as by instructions]({{ site.baseurl }}/build/prerequisites/specific/#hadoop-version) | posix-dir-path| ENV{HADOOP_HOME} |
|SWC_DOCUMENTATION|  configure for generating documentations | ON/OFF | OFF |
|SWC_MALLOC_NOT_INSISTENT|  Not to use SWC-DB insistent malloc | ON/OFF | OFF(clang-ON) |
|SWC_INSTALL_DEP_LIBS|  Install the 3rd-party dependencies libaries used for linking | ON/OFF | OFF |
|SWC_WITHOUT_JAVA| skip java/maven builds | ON/OFF | OFF |
|SWC_ENABLE_SANITIZER| Enable build-wide the specified sanitizer (slower perf. by x3-x10) | address/thread | OFF |
|USE_GLIBC_MALLOC| use compiler malloc | ON/OFF | OFF |
|USE_JEMALLOC| use libjemalloc | ON/OFF | OFF |
|USE_HOARD| use libhoard | ON/OFF | OFF |
|USE_MIMALLOC| use libmimalloc | ON/OFF | OFF |
|USE_TCMALLOC| use libtcmalloc | ON/OFF | OFF(default libtcmalloc_minimal or USE_GLIBC_MALLOC) |
|SWC_LANGUAGES| require to build with support of listed languages  | NONE or ANY / applicable CSV: py2,py3,pypy2,pypy3,java,netstd,c_glib | any possible |
|SWC_BUILTIN_FS| builtin filesystems (impl./prelinked without use of dynamic linking loader), suggested=local,broker | applicable CSV: local,broker,hadoop_jvm,hadoop,ceph | any possible |
|SWC_DEFAULT_ENCODER| the encoder to use for default config value | PLAIN/ZLIB/SNAPPY/ZSTD | ZSTD |
|SWC_BUILD_PKG| Build only the specified package | _Environment:_ <br/> * env  <br/> * doc  <br/> _Libraries:_ <br/> * lib-core <br/>   * lib <br/>   * lib-fs <br/>   * lib-fs-local <br/>   * lib-fs-broker <br/>   * lib-fs-ceph <br/>   * lib-fs-hadoop <br/>   * lib-fs-hadoop-jvm <br/>   * lib-thrift <br/>   * lib-thrift-c <br/>   * pam-max-retries <br/>  _Applications:_ <br/>   * manager <br/>   * ranger <br/>   * fsbroker <br/>   * thiriftbroker <br/>   * utils | NONE(build-all) |
|SWC_PATH_ETC| Build with specific `/etc/` path | posix-dir-path, finish with slash `/` | application-base/../etc/swcdb/ |
|SWC_PATH_LOG| Build with specific `/log/` path | posix-dir-path, finish with slash `/` | application-base/../var/log/swcdb/ |
|SWC_PATH_RUN| Build with specific `/run/` path | posix-dir-path, finish with slash `/` | application-base/../run/ |



## CMake Configuration Options

| CONFIG OPTION | DESCRIPTION | VALUE OPTIONS | DEFAULT VALUE |
| ---  | --- | --- | --- |
|CMAKE_SKIP_RPATH| runtime-linking | ON/OFF | OFF |
|CMAKE_INSTALL_PREFIX| SWC-DB path of installation, suggested /opt/swcdb | posix-dir-path | /usr/local |
|CMAKE_BUILD_TYPE| Build Type (the 'Release' applies NDEBUG to O_LEVEL) | Debug/Release | Debug |



***



## Configuring

*  while at builds [path as by instructions]({{ site.baseurl }}/build/prerequisites/)
```
cd swcdb;
```

_The Configuration Option Format ```-D{option}={value}) ```_

```cmake
cmake ../swc-db [SWC-DB Configuration Options] [Cmake Configuration Options];
```



***



#### Configuration Examples
##### a Release
```bash
    cmake ../swc-db \
      -DO_LEVEL=6 -DSWC_IMPL_SOURCE=ON \
      -DSWC_BUILTIN_FS=local,broker -DSWC_LANGUAGES=ALL \
      -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
      -DWITHOUT_THRIFT_C=OFF \
      -DGLIB_INCLUDE_PATH="$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )" \
      -DWITHOUT_PAM=ON \
      -DCMAKE_SKIP_RPATH=OFF -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
      -DSWC_DOCUMENTATION=OFF \
      -DSWC_INSTALL_DEP_LIBS=ON \
      -DCMAKE_BUILD_TYPE=Release;
```

##### a Debug
```bash
    cmake ../swc-db \
      -DO_LEVEL=1 -DSWC_IMPL_SOURCE=OFF \
      -DSWC_BUILTIN_FS=local,broker -DSWC_LANGUAGES=ALL \
      -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
      -DWITHOUT_THRIFT_C=OFF \
      -DGLIB_INCLUDE_PATH="$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )" \
      -DWITHOUT_PAM=ON \
      -DCMAKE_SKIP_RPATH=OFF -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
      -DSWC_DOCUMENTATION=OFF \
      -DSWC_INSTALL_DEP_LIBS=ON \
      -DCMAKE_BUILD_TYPE=Debug;
```
---
title: Configure
sort: 3
---



# Configuring SWC-DB build

## SWC-DB Configuration Options

| CONFIG OPTION | DESCRIPTION | VALUE OPTIONS | DEFAULT VALUE |
| ---  | --- | --- | --- |
|O_LEVEL| Level of optimizations: <br/>  0: = -Os <br/>  1: = -O2 <br/>  2: = -O2 plus -floop-interchange -flto=1 -fuse-linker-plugin -ffat-lto-objects (GCC; Clang uses -flto) <br/>  3: = -O3 <br/>  4: += -flto=1 -fuse-linker-plugin -ffat-lto-objects<br/>  5: += BUILD_LINKING=STATIC <br/>  6: += BUILD_LINKING_CORE=STATIC | 0-7 | 3 |
|BUILD_LINKING| Link major executables against static or shared dependency libraries. Default **SHARED**. Prefer SHARED when the distro ships only shared deps (e.g. Arch `libre2.so` without `libre2.a`). O_LEVEL 5 forces STATIC. After flipping STATIC↔SHARED, wipe the CMake cache and reconfigure. | STATIC / SHARED | SHARED |
|BUILD_LINKING_CORE| Link core libraries static or shared. O_LEVEL 6 forces STATIC. Same cache note as `BUILD_LINKING`. | STATIC / SHARED | SHARED |
|SWC_IMPL_SOURCE| when possible implement SWC-DB source-code | ON/OFF | OFF |
|SWC_IMPL_COMPARATORS_BASIC| use the basic comparator implementation (`-DSWC_IMPL_COMPARATORS_BASIC`) | ON/OFF | OFF |
|SWC_IO_URING_AS_DEFAULT| enable ASIO io_uring and disable epoll as the default reactor | ON/OFF | OFF |
|SWC_FS_LOCAL_USE_IO_URING| build Local FS with io_uring support (`-DSWC_FS_LOCAL_USE_IO_URING`; also enables ASIO io_uring) | ON/OFF | OFF |
|SWC_RANGER_WITH_RANGEDATA| build Manager/Ranger with RangeData support (`-DSWC_RANGER_WITH_RANGEDATA`) | ON/OFF | OFF |
|SWC_PROFILE| emit stack-usage / profile-oriented compiler flags | STACK / ALL | OFF |
|USE_REPLXX| whether to use Libreplxx | ON/OFF | OFF(ON if found) |
|USE_GNU_READLINE| whether to use GNU libreadline | ON/OFF | OFF(ON if EDITLINE not found) |
|LOOKUP_INCLUDE_PATHS| additional paths to headers | posix-dir-path_LIST; | "/opt/local/include;/usr/local/include;usr/local/lib;/usr/include" |
|LOOKUP_LIB_PATHS| additional paths to libraries | posix-dir-path_LIST; | "/opt/local/lib;/usr/local/lib;/usr/lib;/lib" |
|JAVA_INSTALL_PATH| JAVA_HOME to use, suggested ```$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g)``` | posix-dir-path | ENV{JAVA_HOME} |
|ASIO_INCLUDE_PATH| suggested [as by instructions]({{ site.baseurl }}/build/prerequisites/specific/#version-asio) | posix-dir-path | "" |
|WITHOUT_THRIFT_C| Not to build the libswcdb_thrift_c. Use **ON** for C++-only / CI-like builds: generated Thrift C-Glib sources are compiled under `-Wall -Werror` and unused locals can fail the build. Separate from `SWC_LANGUAGES`. | ON/OFF | OFF |
|GLIB_INCLUDE_PATH| suggested ```$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )``` | posix-dir-path | "" |
|WITHOUT_PAM| Not to build the libpam_swcdb_max_retries | ON/OFF | OFF |
|HADOOP_INSTALL_PATH| HADOOP_HOME to use, suggested [as by instructions]({{ site.baseurl }}/build/prerequisites/specific/#hadoop-version) | posix-dir-path| ENV{HADOOP_HOME} |
|SWC_DOCUMENTATION|  configure for generating documentations | ON/OFF | OFF |
|SWC_MALLOC_NOT_INSISTENT|  Not to use SWC-DB insistent malloc | ON/OFF | OFF(clang-ON) |
|SWC_INSTALL_DEP_LIBS|  Install the 3rd-party dependencies libaries used for linking | ON/OFF | OFF |
|SWC_WITHOUT_JAVA| skip java/maven builds | ON/OFF | OFF |
|SWC_ENABLE_SANITIZER| Enable build-wide the specified sanitizer (slower perf. by x3-x10) | address/thread | OFF |
|USE_DEFAULT_MALLOC| use compiler/libc malloc | ON/OFF | OFF |
|USE_JEMALLOC| use libjemalloc | ON/OFF | OFF |
|USE_HOARD| use libhoard | ON/OFF | OFF |
|USE_MIMALLOC| use libmimalloc | ON/OFF | OFF |
|USE_TCMALLOC| use libtcmalloc | ON/OFF | OFF(default libtcmalloc_minimal or USE_DEFAULT_MALLOC) |
|USE_LIBSSL| The SSL-Library to use | open/wolf | open |
|SWC_LANGUAGES| require to build with support of listed languages. CSV values are handled by `FindLanguages.cmake`: `py2`, `py3`, `pypy2`, `pypy3`, `ruby`, `java`. Use `ALL` (or leave unset) for every available language binding. C-Glib Thrift is controlled by `WITHOUT_THRIFT_C` / `SWC_BUILD_PKG=lib-thrift-c`, not this CSV. Netstd generation is part of a full/ALL Thrift language build, not a CSV token. | NONE / ALL / CSV: py2,py3,pypy2,pypy3,ruby,java | any possible |
|SWC_BUILTIN_FS| builtin filesystems (impl./prelinked without use of dynamic linking loader), suggested=local,broker | applicable CSV: local,broker,hadoop_jvm,hadoop,ceph | any possible |
|SWC_DEFAULT_ENCODER| the encoder to use for default config value | PLAIN/ZLIB/SNAPPY/ZSTD | ZSTD |
|SWC_BUILD_PKG| Build only the specified package | _Environment:_ <br/> * env  <br/> * doc  <br/> _Libraries:_ <br/> * lib-core <br/>   * lib <br/>   * lib-fs <br/>   * lib-fs-local <br/>   * lib-fs-broker <br/>   * lib-fs-ceph <br/>   * lib-fs-hadoop <br/>   * lib-fs-hadoop-jvm <br/>   * lib-thrift <br/>   * lib-thrift-c <br/>   * pam (any value matching `^pam`, e.g. `pam-max-retries`) <br/>  _Applications:_ <br/>   * manager <br/>   * ranger <br/>   * broker <br/>   * fsbroker <br/>   * thriftbroker <br/>   * utils | NONE(build-all) |
|SWC_PATH_ETC| Build with specific `/etc/` path | posix-dir-path, finish with slash `/` | application-base/../etc/swcdb/ |
|SWC_PATH_LOG| Build with specific `/log/` path | posix-dir-path, finish with slash `/` | application-base/../var/log/swcdb/ |
|SWC_PATH_RUN| Build with specific `/run/` path | posix-dir-path, finish with slash `/` | application-base/../run/ |
|SWC_WITHOUT_THRIFT| skip Thrift builds |  ON/OFF | OFF |



## CMake Configuration Options

| CONFIG OPTION | DESCRIPTION | VALUE OPTIONS | DEFAULT VALUE |
| ---  | --- | --- | --- |
|CMAKE_SKIP_RPATH| runtime-linking | ON/OFF | OFF |
|CMAKE_INSTALL_PREFIX| SWC-DB path of installation. Examples and packaged layouts use **`/opt/swcdb`**; CMake's own default remains `/usr/local` if unset. | posix-dir-path | /usr/local |
|CMAKE_BUILD_TYPE| Build Type (the 'Release' applies NDEBUG to O_LEVEL) | Debug/Release | Debug |



***



## Configuring

Pick a build directory layout, then run `cmake` with `-D{option}={value}`:

| Layout | From build directory | Typical command |
| --- | --- | --- |
| Sibling tree (`builds/swcdb` next to a `swc-db` checkout) | `cd builds/swcdb` | `cmake ../swc-db …` |
| Nested under the checkout (`swc-db/builds/swcdb`) | `cd builds/swcdb` | `cmake ../../swc-db …` |

* while at builds [path as by instructions]({{ site.baseurl }}/build/prerequisites/)
```
cd builds/swcdb;
```

_The Configuration Option Format ```-D{option}={value}```_

```cmake
cmake ../swc-db [SWC-DB Configuration Options] [Cmake Configuration Options];
```
> Adjust the relative path (`../swc-db` vs `../../swc-db`) to match the layout above. Prefer `-DCMAKE_INSTALL_PREFIX=/opt/swcdb` so installed binaries match the Getting Started and run guides.


***



#### Configuration Examples
##### CI-like smoke / unit-test build (C++ only)
Matches default GitHub Actions configure habits: no language bindings, shared linking, install under `/opt/swcdb`. Good first configure before a full `SWC_LANGUAGES=ALL` build. Leave `SWC_BUILD_PKG` unset (empty) so tests and examples are built.

```bash
    cmake ../swc-db \
      -DO_LEVEL=3 -DSWC_IMPL_SOURCE=OFF \
      -DBUILD_LINKING=SHARED \
      -DSWC_LANGUAGES=NONE \
      -DWITHOUT_THRIFT_C=ON \
      -DWITHOUT_PAM=ON \
      -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
      -DCMAKE_SKIP_RPATH=OFF \
      -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
      -DSWC_DOCUMENTATION=OFF \
      -DCMAKE_BUILD_TYPE=Debug;
```

##### an Optimized Release build
```bash
    cmake ../swc-db \
      -DO_LEVEL=6 -DSWC_IMPL_SOURCE=ON \
      -DSWC_BUILTIN_FS=local,broker \
      -DSWC_LANGUAGES=ALL \
      -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
      -DWITHOUT_THRIFT_C=OFF \
      -DGLIB_INCLUDE_PATH="$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )" \
      -DWITHOUT_PAM=ON \
      -DCMAKE_SKIP_RPATH=OFF -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
      -DSWC_DOCUMENTATION=OFF \
      -DCMAKE_BUILD_TYPE=Release;
```

##### a Standard Release build
```bash
    cmake ../swc-db \
      -DO_LEVEL=3 -DSWC_IMPL_SOURCE=OFF \
      -DSWC_LANGUAGES=ALL \
      -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
      -DWITHOUT_THRIFT_C=OFF \
      -DGLIB_INCLUDE_PATH="$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )" \
      -DWITHOUT_PAM=ON \
      -DCMAKE_SKIP_RPATH=OFF \
      -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
      -DSWC_DOCUMENTATION=OFF \
      -DCMAKE_BUILD_TYPE=Release;
```

##### a Debug Release build
```bash
    cmake ../swc-db \
      -DO_LEVEL=1 -DSWC_IMPL_SOURCE=OFF \
      -DSWC_LANGUAGES=ALL \
      -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
      -DWITHOUT_THRIFT_C=OFF \
      -DGLIB_INCLUDE_PATH="$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )" \
      -DWITHOUT_PAM=ON \
      -DCMAKE_SKIP_RPATH=OFF \
      -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
      -DSWC_DOCUMENTATION=OFF \
      -DCMAKE_BUILD_TYPE=Debug;
```
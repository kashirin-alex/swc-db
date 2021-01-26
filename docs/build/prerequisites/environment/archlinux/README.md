---
title: Archlinux
sort: 2
---

# Archlinux


In order to build SWC-DB on Archlinux there are several mandatory and optional prerequisites which are available from the os-distribution or some might require additional steps.


## SWC-DB Mandatory 3rd-party Prerequisites:
```bash
pacman -Syu;
```
```bash
pacman -S \
  make \
  cmake \
  pkg-config \
  gcc \
  asio \
  snappy \
  re2;
```

##### REQUIRED VERSIONS:
  * [ASIO]({{ site.baseurl }}/build/prerequisites/specific/#version-asio)


***


## SWC-DB Optional 3rd-party Prerequisites:

### Malloc

#### libtcmalloc:
```bash
pacman -S gperftools;
```


### Thrift Broker
```bash
pacman -S \
  boost \
  libevent \
  thrift;
```


### Java-Thrift client
```bash
pacman -S \
  jdk11-openjdk \
  maven;
```

### C_GLIB-Thrift client & PAM module
```bash
pacman -S \
  glib2 \
  pam;
```


### Hadoop FsBroker native(JVM, C++)

* [HADOOP VERSION]({{ site.baseurl }}/build/prerequisites/specific/#hadoop-version)

_tests require full Java & Apache-Hadoop installation and runtime_

   * #### Native(JVM)
      ```bash
      pacman -S jdk11-openjdk
      ```
     _Version rather than JAVA_HOME apply cmake with `-DJAVA_INSTALL_PATH=$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g);`_

   * #### Native(C++)
      ```bash
      pacman -S protobuf;
      ```
     _state depends on progress of [libhdfscpp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)_



### CephFS
```bash

pacman -S ceph-libs;
```


### Documentations
```bash
pacman -S graphviz doxygen;
```
Generating Thrift Documentations - [COMPILER THRIFT]({{ site.baseurl }}/build/prerequisites/specific/#compiler-thrift)



### swcdb_cluster
```
python3 -m ensurepip;
python3 -m pip install setuptools;
python3 -m pip install fabric;
```


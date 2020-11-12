---
title: Debian/Ubuntu
sort: 1 
---

# Debian/Ubuntu


In order to build SWC-DB on Debian/Ubuntu there are several mandatory and optional prerequisites which are available from the os-distribution or some might require additional steps.


## SWC-DB Mandatory 3rd-party Prerequisites: 
```bash
apt-get update;
```

```bash
apt-get install -y \
  cmake \
  build-essential \
  libasio-dev \
  libre2-dev \
  libz-dev \
  libsnappy-dev \
  libzstd-dev \
  libssl-dev \
  libreadline-dev \
  git;
```

##### REQUIRED VERSIONS:
  * [ASIO]({{ site.baseurl }}/build/prerequisites/specific/#version-asio) 


***


## SWC-DB Optional 3rd-party Prerequisites:

### Malloc

#### libtcmalloc:
```bash
apt-get install -y libgoogle-perftools-dev;
```


### Thrift Broker
```bash
apt-get install -y 
  libboost-math-dev \
  libevent-dev \
  libthrift-dev;
```


### Java-Thrift client
```bash
apt-get install -y \
  default-jdk \
  maven;
```

### C_GLIB-Thrift client & PAM module
```bash
apt-get install -y 
  libthrift-c-glib-dev \
  libperl-dev \
  libffi-dev \
  libglib2.0-dev \
  libpam-dev;
```


### Hadoop FsBroker native(JVM, C++)

* [HADOOP VERSION]({{ site.baseurl }}/build/prerequisites/specific/#hadoop-version) 

_tests require full Java & Apache-Hadoop installation and runtime_

   * #### Native(JVM)
      ```bash
      apt-get install -y default-jdk;
      ```
     _Version rather than JAVA_HOME apply cmake with `-DJAVA_INSTALL_PATH=$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g);`_

   * #### Native(C++)
      ```bash
      apt-get install -y libprotobuf-dev;
      ```
     _state depends on progress of [libhdfscpp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)_



### CephFS
```bash

apt-get install -y libcephfs-dev;
```


### Documentations
```bash
apt-get install -y graphviz doxygen;
```
Generating Thrift Documentations - [COMPILER THRIFT]({{ site.baseurl }}/build/prerequisites/specific/#compiler-thrift)



### swcdb_cluster
```
apt-get install -y python3-pip;
python3 -m pip install setuptools;
python3 -m pip install fabric;
```


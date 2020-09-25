---
title: Prerequisites
sort: 2
---


## Prepare builds path
```bash
mkdir builds; 
cd builds;
```


## Get the SWC-DB source:

```bash
git clone https://github.com/kashirin-alex/swc-db.git;
mkdir swcdb; 
```


## 3rd-party Prerequisites:

### SWC-DB Mandatory 3rd-party Prerequisites: 

  * [cmake 3.13+](https://cmake.org/)

  * [gcc 9+](https://gcc.gnu.org/) 
    | 
    [clang 9+](http://llvm.org/)

  * [ASIO 1.18+](https://github.com/chriskohlhoff/asio)

  * [libre2](https://github.com/google/re2)

  * [libsnappy](https://github.com/google/snappy)

  * [libzlib](https://www.zlib.net/)

  * [libzstd](https://github.com/facebook/zstd)

  * [libopenssl TLS-1.3](https://www.openssl.org/)

  * [libeditline](https://github.com/troglobit/editline) 
    | 
    [libreadline](https://tiswww.case.edu/php/chet/readline/rltop.html)




***




### SWC-DB Optional 3rd-party Prerequisites:
  * Malloc 
    * [libtcmalloc](https://github.com/gperftools/gperftools) 
      |
      [libjemalloc](http://github.com/jemalloc/jemalloc)
      |
      [libhoard](http://github.com/emeryberger/Hoard/)

  * [Java(openjdk 12.0.1)](https://jdk.java.net/java-se-ri/12) - [Hadoop-JVM FsBroker, Java-Thrift client]

  * [maven](https://maven.apache.org/) - [Java-Thrift client]

  * Thrift Broker:
    * [Apache Thrift 0.13.0+](https://github.com/apache/thrift)
    * [libevent 2.1.11+](http://github.com/libevent/libevent)
  
  * C_GLIB-Thrift client & PAM module:
    * [libglib-2.0](https://developer.gnome.org/glib/2.64/)
    * [libgobject-2.0](https://developer.gnome.org/gobject/2.64/)
    * [libffi](http://github.com/libffi/libffi/)
    * [libpcre 1](https://pcre.org/)
  
  * Hadoop-JVM FsBroker:
    * [Apache-Hadoop + libhdfs](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfs)
  
  * Hadoop FsBroker:
    * [Apache-Hadoop + libhdfspp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)
    * [libprotobuf](https://github.com/protocolbuffers/protobuf)
  
  * CephFS FsBroker
    * [libcephfs](https://docs.ceph.com/en/latest/cephfs/)

  * [fabric-pylib](https://github.com/fabric/fabric) - (sbin/swcdb_cluster)
  
  * [doxygen](https://github.com/doxygen/doxygen)
  


***


_**The following Environments with installation/setup information of prerequisites are available:**_

* [Debian/Ubuntu](environment/debian_ubuntu/)
* [Linux From Scratch](environment/linux_from_scratch/)



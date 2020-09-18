
# Linux From Scratch

## SWC-DB Mandatory 3rd-party Prerequisites: 

##### REQUIRED VERSIONS:
  * [ASIO](/swc-db//build/prerequisites/specific/version_asio) 


***


## SWC-DB Optional 3rd-party Prerequisites:

### Malloc

#### libtcmalloc:


### Thrift Broker


### Java-Thrift client

### C_GLIB-Thrift client & PAM module


### Hadoop FsBroker native(JVM, C++)

* [HADOOP VERSION](/swc-db/build/prerequisites/specific/version_hadoop) 

_tests require full Java & Apache-Hadoop installation and runtime_

   * #### Native(JVM)
     _Version rather than JAVA_HOME apply cmake with `-DJAVA_INSTALL_PATH=$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g);`_

   * #### Native(C++)
     _state depends on progress of [libhdfscpp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)_



### CephFS


### Documentations
Generating Thrift Documentations - [COMPILER THRIFT](/swc-db//build/prerequisites/specific/compiler_thrift)



### swcdb_cluster
```
python3 -m pip install setuptools;
python3 -m pip install fabric;
```


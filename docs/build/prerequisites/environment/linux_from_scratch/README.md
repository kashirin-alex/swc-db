
# Linux From Scratch

## SWC-DB Mandatory 3rd-party Prerequisites: 

##### REQUIRED VERSIONS:
  * [ASIO]({{ site.baseurl }}/build/prerequisites/specific/#version-asio) 


***


## SWC-DB Optional 3rd-party Prerequisites:

### Malloc

#### libtcmalloc:


### Thrift Broker


### Java-Thrift client

### C_GLIB-Thrift client & PAM module


### Hadoop FsBroker native(JVM, C++)

* [HADOOP VERSION]({{ site.baseurl }}/build/prerequisites/specific/#version-hadoop) 

_tests require full Java & Apache-Hadoop installation and runtime_

   * #### Native(JVM)
     _Version rather than JAVA_HOME apply cmake with `-DJAVA_INSTALL_PATH=$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g);`_

   * #### Native(C++)
     _state depends on progress of [libhdfscpp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)_



### CephFS


### Documentations
Generating Thrift Documentations - [COMPILER THRIFT]({{ site.baseurl }}/build/prerequisites/specific/#compiler-thrift)



### swcdb_cluster
```
python3 -m pip install setuptools;
python3 -m pip install fabric;
```


---
title: Hadoop-JVM Filesystem
---



# The SWC-DB Hadoop-JVM Filesystem - Configuration Properties
The SWC-DB Hadoop-JVM Filesystem Library can be configured with these configuration properties.

* ### swc.fs.hadoop_jvm.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.hadoop_jvm.cfg.dyn=```**

* ### swc.fs.hadoop_jvm.path.root
```STRING```
The Hadoop-JVM FileSystem's base root path. \
_default_ **```swc.fs.hadoop_jvm.path.root=```**


* ### swc.fs.hadoop_jvm.namenode
```STRINGS```
The Namenode Host + optionally the :Port, many key=value allowed. 
> The hadoop ```.xml``` confgiurations are used, if not set.

  _default_ **```swc.fs.hadoop_jvm.namenode=```**

* ### swc.fs.hadoop_jvm.namenode.port
```INT16```
The Namenode Port, usefull if many namenodes and port is the same. \
_default_ **```swc.fs.hadoop_jvm.namenode.port=```**


* ### swc.fs.hadoop_jvm.user
```STRING```
Use Hadoop-JVM under this username. \
_default_ **```swc.fs.hadoop_jvm.user=```**


 > ***
 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.hadoop_jvm.fds.max
```G_INT32```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.hadoop_jvm.fds.max=1024```**


* ### swc.fs.hadoop_jvm.reconnect.delay.ms
```G_INT32```
The time in milliseconds to delay use of a connection after re-connect. \
_default_ **```swc.fs.hadoop_jvm.reconnect.delay.ms=3000```**



---
title: Hadoop-JVM Filesystem
---



# The SWC-DB Hadoop-JVM Filesystem - Configuration Properties
The SWC-DB Hadoop-JVM Filesystem Library can be configured with these configuration properties.

* ### swc.fs.hadoop_jvm.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.hadoop_jvm.cfg.dyn=```**

* ### swc.fs.hadoop_jvm.path.root
```TYPE_STRING```
The Hadoop-JVM FileSystem's base root path. \
_default_ **```swc.fs.hadoop_jvm.path.root=```**

* ### swc.fs.hadoop_jvm.metrics.enabled
```TYPE_BOOL```
Enable or Disable Metrics tracking. \
_default_ **```swc.fs.hadoop_jvm.metrics.enabled=true```**


* ### swc.fs.hadoop_jvm.namenode
```TYPE_STRINGS```
The Namenode Host + optionally the :Port, many key=value allowed.
> The hadoop ```.xml``` confgiurations are used, if not set.

  _default_ **```swc.fs.hadoop_jvm.namenode=```**

* ### swc.fs.hadoop_jvm.namenode.port
```TYPE_UINT16```
The Namenode Port, usefull if many namenodes and port is the same. \
_default_ **```swc.fs.hadoop_jvm.namenode.port=```**


* ### swc.fs.hadoop_jvm.user
```TYPE_STRING```
Use Hadoop-JVM under this username. \
_default_ **```swc.fs.hadoop_jvm.user=```**


 > ***
 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.hadoop_jvm.fds.max
```TYPE_INT32_G```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.hadoop_jvm.fds.max=1024```**


* ### swc.fs.hadoop_jvm.reconnect.delay.ms
```TYPE_INT32_G```
The time in milliseconds to delay use of a connection after re-connect. \
_default_ **```swc.fs.hadoop_jvm.reconnect.delay.ms=3000```**


* ### swc.fs.hadoop_jvm.read.buffer.size
```TYPE_INT32_G```
The size of read buffer in bytes. \
_default_ **```swc.fs.hadoop_jvm.read.buffer.size=0```**

* ### swc.fs.hadoop_jvm.write.buffer.size
```TYPE_INT32_G```
The size of write buffer in bytes. \
_default_ **```swc.fs.hadoop_jvm.write.buffer.size=0```**


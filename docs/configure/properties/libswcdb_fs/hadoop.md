---
title: Hadoop Filesystem
---



# The SWC-DB Hadoop Filesystem - Configuration Properties
The SWC-DB Hadoop Filesystem Library can be configured with these configuration properties.

* ### swc.fs.hadoop.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.hadoop.cfg.dyn=```**

* ### swc.fs.hadoop.path.root
```STRING```
The Hadoop FileSystem's base root path. \
_default_ **```swc.fs.hadoop.path.root=```**


* ### swc.fs.hadoop.namenode
```STRINGS```
The Namenode Host + optionally the :Port, many key=value allowed. 
> The hadoop ```.xml``` confgiurations are used, if not set.

  _default_ **```swc.fs.hadoop.namenode=```**

* ### swc.fs.hadoop.namenode.port
```INT16```
The Namenode Port, usefull if many namenodes and port is the same. \
_default_ **```swc.fs.hadoop.namenode.port=```**


* ### swc.fs.hadoop.user
```STRING```
Use Hadoop under this username. \
_default_ **```swc.fs.hadoop.user=```**

* ### swc.fs.hadoop.handlers
```INT32```
The number of handlers for hadoop tasks. \
_default_ **```swc.fs.hadoop.handlers=48```**


 > ***
 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.hadoop.fds.max
```G_INT32```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.hadoop.fds.max=1024```**

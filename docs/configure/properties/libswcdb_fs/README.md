---
title: Filesystem Library 
sort: 1
---




# The SWC-DB FS Library Configuration Properties
  The SWC-DB filesystem configuration properties are defined in two spaces The Filesystem Base Library (```libswcdb_fs```) and Filesystem Type Library (```libswcdb_fs_TYPE```).
  
  **_Configuration Properties by Filesystem Type/Library:_**
  * [The Filesystem Base Library](#the-filesystem-base-library---configuration-properties)
  * [The Local Filesystem Library](local.html)
  * [The Hadoop Filesystem Library](hadoop.html)
  * [The Hadoop-JVM Filesystem Library](hadoop_jvm.html)
  * [The Ceph Filesystem Library](ceph.html)
  * [The Broker Filesystem Library](broker.html)


***



## The Filesystem Base Library - Configuration Properties
These properties are the filesystem base library configuration properties, The Base properties are applied/used/defined before the Filesystem Type Library configuration properties.


* ### swc.fs.path.data
```STRING```
The data path, the path is relative to the Type of Filesystem defined in ```swc.fs.TYPE.path.root```. The folders for Columns will be created on this path. \
_default_ **```swc.fs.path.data=swcdb/```**

* ### swc.fs
```STRING```
The main FileSystem type: ```local``` ```hadoop``` ```hadoop_jvm``` ```ceph``` ```broker``` ```custom``` 'custom' let use of a custom made FS Library based on the SWC-DB FS Library (```libswcdb_fs```). SWC-DB Programs will work with the configured Filesystem as the main Filesystem.
> By using local means it is not a distributed filesystem and the SWC-DB run can be only the local [Psedumode](/run/psedomode/).
  
  _default_ **```swc.fs=```**




> **_The configuration properties specific to Local Filesystem_**

* ### swc.fs.local.cfg
```STRING```
The specific cfg-file of the Local Filesystem. \
 _default_ **```swc.fs.local.cfg=```**

* ### swc.fs.lib.local
```STRING```
The Local Library file to use, if and Local is not a builtin filesystem. \
If not set the default lib is _SWC_INSTALL_PREFIX/lib/libswcdb_fs_local.so_ \
_default_ **```swc.fs.lib.local=```**



> **_The configuration properties specific to Hadoop Filesystem_**

* ### swc.fs.hadoop.cfg
```STRING```
The specific cfg-file of the Hadoop Filesystem. \
_default_ **```swc.fs.hadoop.cfg=```**

* ### swc.fs.lib.hadoop
```STRING```
The Hadoop Library file to use, if and Hadoop is not a builtin filesystem. \
If not set the default lib is _SWC_INSTALL_PREFIX/lib/libswcdb_fs_hadoop.so_ \
_default_ **```swc.fs.lib.hadoop=```**



> **_The configuration properties specific to Hadoop-JVM Filesystem_**

* ### swc.fs.hadoop_jvm.cfg
```STRING```
The specific cfg-file of the Hadoop-JVM Filesystem. \
_default_ **```swc.fs.hadoop_jvm.cfg=```**

* ### swc.fs.lib.hadoop_jvm
```STRING```
The Hadoop-JVM Library file to use, if and Hadoop-JVM is not a builtin filesystem. \
If not set the default lib is _SWC_INSTALL_PREFIX/lib/libswcdb_fs_hadoop_jvm.so_ \
_default_ **```swc.fs.lib.hadoop_jvm=```**



> **_The configuration properties specific to Ceph Filesystem_**

* ### swc.fs.ceph.cfg
```STRING```
The specific cfg-file of the Ceph Filesystem. \
_default_ **```swc.fs.ceph.cfg=```**

* ### swc.fs.lib.ceph
```STRING```
The Ceph Library file to use, if and Ceph is not a builtin filesystem. \
If not set the default lib is _SWC_INSTALL_PREFIX/lib/libswcdb_fs_ceph.so_ \
_default_ **```swc.fs.lib.ceph=```**



> **_The configuration properties specific to Custom Filesystem_**

* ### swc.fs.custom.cfg
```STRING```
The specific cfg-file of the Custom Filesystem. \
_default_ **```swc.fs.custom.cfg=```**

* ### swc.fs.lib.custom
```STRING```
The Custom Library file to use. \
_default_ **```swc.fs.lib.custom=```** 



> **_The configuration properties specific to Broker Filesystem_**

* ### swc.fs.broker.cfg
```STRING```
The specific cfg-file of the Broker Filesystem. \
_default_ **```swc.fs.broker.cfg=```**

* ### swc.fs.lib.broker
```STRING```
The Broker Library file to use. \
_default_ **```swc.fs.lib.broker=```** 

* ### swc.fs.broker.underlying
```STRING```
The FsBroker's underlying FileSystem type: ```local``` ```hadoop``` ```hadoop_jvm``` ```ceph``` ```custom``` as ```swc.fs``` property but without ```broker``` .\
_default_ **```swc.fs.broker.underlying=```**



***

> **_The configuration properties applicable for dynamic reloading_**





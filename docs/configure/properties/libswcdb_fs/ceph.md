---
title: Ceph Filesystem
---



# The SWC-DB Ceph Filesystem - Configuration Properties
The SWC-DB Ceph Filesystem Library can be configured with these configuration properties.

* ### swc.fs.ceph.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.ceph.cfg.dyn=```**

* ### swc.fs.ceph.path.root
```TYPE_STRING```
The Ceph FileSystem's base root path. \
_default_ **```swc.fs.ceph.path.root=```**

* ### swc.fs.ceph.metrics.enabled
```TYPE_BOOL```
Enable or Disable Metrics tracking. \
_default_ **```swc.fs.ceph.metrics.enabled=true```**

* ### swc.fs.ceph.client.id
```TYPE_STRING```
The CephFs Client-Id to use. \
_default_ **```swc.fs.ceph.client.id=```**

* ### swc.fs.ceph.configuration.file
```TYPE_STRING```
The CephFs configuration file ```ceph.conf``` path. \
_default_ **```swc.fs.ceph.configuration.file=```**


  > ***
  > _The available properties to apply if ```swc.fs.ceph.configuration.file``` is not set_

* ### swc.fs.ceph.mon.addr
```TYPE_STRING```
The CephFs monitor-address ```mon_addr```. \
_default_ **```swc.fs.ceph.mon.addr=```**


* ### swc.fs.ceph.perm.user
```TYPE_INT32```
The CephFs Permission Group. \
_default_ **```swc.fs.ceph.perm.group=```**

* ### swc.fs.ceph.perm.user
```TYPE_INT32```
The CephFs Permission User. \
_default_ **```swc.fs.ceph.perm.user=```**


* ### swc.fs.ceph.stripe.unit
```TYPE_INT32```
The CephFs default stripe_unit. \
_default_ **```swc.fs.ceph.stripe.unit=```**

* ### swc.fs.ceph.stripe.count
```TYPE_INT32```
The CephFs default stripe_count. \
_default_ **```swc.fs.ceph.stripe.count=```**

* ### swc.fs.ceph.object.size
```TYPE_INT32```
The CephFs default object_size. \
_default_ **```swc.fs.ceph.object.size=```**

* ### swc.fs.ceph.replication
```TYPE_INT32```
The CephFs default replication. \
_default_ **```swc.fs.ceph.replication=```**


 > ***
 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.ceph.fds.max
```TYPE_INT32_G```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.ceph.fds.max=256```**


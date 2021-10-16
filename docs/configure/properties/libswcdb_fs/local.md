---
title: Local Filesystem
---



# The SWC-DB Local Filesystem - Configuration Properties
The SWC-DB Local Filesystem Library can be configured with these configuration properties.

* ### swc.fs.local.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.local.cfg.dyn=```**

* ### swc.fs.local.path.root
```TYPE_STRING```
The Local FileSystem's base root path. \
_default_ **```swc.fs.local.path.root=```**

* ### swc.fs.local.metrics.enabled
```TYPE_BOOL```
Enable or Disable Metrics tracking. \
_default_ **```swc.fs.local.metrics.enabled=true```**


 > ***
 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.local.fds.max
```TYPE_INT32_G```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.local.fds.max=1024```**


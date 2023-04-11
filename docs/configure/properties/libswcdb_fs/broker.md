---
title: Broker Filesystem
---



# The SWC-DB Broker Filesystem - Configuration Properties
The SWC-DB Broker Filesystem Library can be configured with these configuration properties.

* ### swc.fs.broker.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.broker.cfg.dyn=```**

* ### swc.fs.broker.host
```TYPE_STRING```
The FsBroker host (default by hostname resolution). \
_default_ **```swc.fs.broker.host=```**

* ### swc.fs.broker.port
```TYPE_UINT16```
The FsBroker port. \
_default_ **```swc.fs.broker.port=14000```**

* ### swc.fs.broker.concurrency.relative
```TYPE_BOOL```
Whether HW-Concurrency base is used with the Applicable cfg properties. \
_default_ **```swc.fs.broker.concurrency.relative=true```**

* ### swc.fs.broker.handlers
```TYPE_INT32```
Number or HW-Concurrency base of Handlers for broker tasks. \
_default_ **```swc.fs.broker.handlers=6```**

* ### swc.fs.broker.metrics.enabled
```TYPE_BOOL```
Enable or Disable Metrics tracking. \
_default_ **```swc.fs.broker.metrics.enabled=true```**


***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.broker.fds.max
```TYPE_INT32_G```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.broker.fds.max=256```**

* ### swc.fs.broker.timeout
```TYPE_INT32_G```
The Default request timeout in milliseconds. \
_default_ **```swc.fs.broker.timeout=120000```**

* ### swc.fs.broker.timeout.bytes.ratio
```TYPE_INT32_G```
The Timeout ratio to bytes, ``` bytes / ratio = milliseconds ``` added to ```swc.fs.broker.timeout```(default timeout). \
_default_ **```swc.fs.broker.timeout.bytes.ratio=1000```**

* ### swc.fs.broker.comm.encoder
```TYPE_ENUM_G```
The encoding to use in communication with FsBroker, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.fs.broker.comm.encoder=ZSTD```**

---
title: Broker Filesystem
---



# The SWC-DB Broker Filesystem - Configuration Properties
The SWC-DB Broker Filesystem Library can be configured with these configuration properties.

* ### swc.fs.broker.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. Config Handler will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.fs.broker.cfg.dyn=```**

* ### swc.fs.broker.host
```STRING```
The FsBroker host (default by hostname resolution). \
_default_ **```swc.fs.broker.host=```**

* ### swc.fs.broker.port
```INT16```
The FsBroker port. \
_default_ **```swc.fs.broker.port=17000```**

* ### swc.fs.broker.handlers
```INT32```
The Handlers for broker tasks. \
_default_ **```swc.fs.broker.handlers=48```**

* ### swc.fs.broker.metrics.enabled
```BOOL```
Enable or Disable Metrics tracking. \
_default_ **```swc.fs.broker.metrics.enabled=true```**


***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.fs.broker.fds.max
```G_INT32```
The Max Open File Descriptors for the option of not closing, Condition dependable by the Program using the filesystem. \
_default_ **```swc.fs.broker.fds.max=256```**

* ### swc.fs.broker.timeout
```G_INT32```
The Default request timeout in milliseconds. \
_default_ **```swc.fs.broker.timeout=120000```**

* ### swc.fs.broker.timeout.bytes.ratio
```G_INT32```
The Timeout ratio to bytes, ``` bytes / ratio = milliseconds ``` added to ```swc.fs.broker.timeout```(default timeout). \
_default_ **```swc.fs.broker.timeout.bytes.ratio=1000```**

* ### swc.fs.broker.comm.encoder
```G_ENUM```
The encoding to use in communication with FsBroker, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.fs.broker.comm.encoder=ZSTD```**

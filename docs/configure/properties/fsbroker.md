---
title: FsBroker Configuration
---



# The SWC-DB FsBroker - Configuration Properties



* ### swc.FsBroker.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. FsBrokers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.FsBroker.cfg.dyn=```**

* ### swc.FsBroker.reactors
```INT32```
The number of communication reactor to initialize an io-context for a given acceptor-fd. \
_default_ **```swc.FsBroker.reactors=8```**

* ### swc.FsBroker.workers
```INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.FsBroker.workers=32```**

* ### swc.FsBroker.handlers
```INT32```
The number of Application handlers. \
_default_ **```swc.FsBroker.handlers=8```**

* ### swc.FsBroker.metrics.enabled
```BOOL```
Enable or Disable FsBroker Metrics Monitoring. \
_default_ **```swc.FsBroker.metrics.enabled=true```**

* ### swc.fs.broker.host
```STRING```
The FsBroker host (default resolve by hostname). \
_default_ **```swc.fs.broker.host=```**

* ### swc.fs.broker.port
```INT16```
The FsBroker port. \
_default_ **```swc.fs.broker.port=14000```**



***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.FsBroker.comm.encoder
```G_ENUM```
The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

_default_ **```swc.FsBroker.comm.encoder=ZSTD```**

* ### swc.FsBroker.metrics.report.interval
```G_INT32```
The Metrics Reporting Interval in Seconds. Zero(=0) skips metrics update.\
_default_ **```swc.FsBroker.metrics.report.interval=300```**



***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./swcdbFsBroker --help;
```

```
./swcdbFsBroker --help-config;
```

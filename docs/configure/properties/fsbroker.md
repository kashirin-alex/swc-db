---
title: FsBroker Configuration
---



# The SWC-DB FsBroker - Configuration Properties



* ### swc.FsBroker.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. FsBrokers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.FsBroker.cfg.dyn=```**

* ### swc.FsBroker.concurrency.relative
```TYPE_BOOL```
Whether HW-Concurrency base is used with the Applicable cfg properties. \
_default_ **```swc.FsBroker.concurrency.relative=true```**

* ### swc.FsBroker.reactors
```TYPE_INT32```
The number of communication reactor to initialize an io-context for a given acceptor-fd.
The total or the base of HW-concurrency for one reactor. \
_default_ **```swc.FsBroker.reactors=4```**

* ### swc.FsBroker.workers
```TYPE_INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.FsBroker.workers=16```**

* ### swc.FsBroker.handlers
```TYPE_INT32```
The number or HW-Concurrency base of Application handlers. \
_default_ **```swc.FsBroker.handlers=32```**

* ### swc.FsBroker.metrics.enabled
```TYPE_BOOL```
Enable or Disable FsBroker Metrics Monitoring. \
_default_ **```swc.FsBroker.metrics.enabled=true```**

* ### swc.FsBroker.metrics.report.broker
```TYPE_BOOL```
Report Metrics via Broker Client. \
_default_ **```swc.FsBroker.metrics.report.broker=true```**

* ### swc.fs.broker.host
```TYPE_STRING```
The FsBroker host (default resolve by hostname). \
_default_ **```swc.fs.broker.host=```**

* ### swc.fs.broker.port
```TYPE_UINT16```
The FsBroker port. \
_default_ **```swc.fs.broker.port=14000```**



***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.FsBroker.comm.encoder
```TYPE_ENUM_G```
The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

_default_ **```swc.FsBroker.comm.encoder=ZSTD```**

* ### swc.FsBroker.metrics.report.interval
```TYPE_INT32_G```
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

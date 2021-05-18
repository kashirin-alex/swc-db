---
title: Broker Configuration
---



# The SWC-DB Broker - Configuration Properties



* ### swc.bkr.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. Brokers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.bkr.cfg.dyn=```**

* ### swc.bkr.reactors
```INT32```
The number of communication reactor to initialize an io-context for a given acceptor-fd. \
_default_ **```swc.bkr.reactors=1```**

* ### swc.bkr.workers
```INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.bkr.workers=32```**

* ### swc.bkr.handlers
```INT32```
The number of Application handlers. \
_default_ **```swc.bkr.handlers=8```**

* ### swc.bkr.clients.handlers
```INT32```
The number of DB-Clients handlers. \
_default_ **```swc.bkr.clients.handlers=8```**

* ### swc.bkr.metrics.enabled
```BOOL```
Enable or Disable Broker Metrics Monitoring. \
_default_ **```swc.bkr.metrics.enabled=true```**


***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.bkr.comm.encoder
```G_ENUM```
The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.mngr.comm.encoder=ZSTD```**

* ### swc.bkr.ram.allowed.percent
```G_INT32```
The Memory RSS % allowed without freeing/releasing. \
_default_ **```swc.bkr.ram.allowed.percent=33```**

* ### swc.bkr.ram.reserved.percent
```G_INT32```
The Memory Total % reserved, threshold of low-memory enter state. \
_default_ **```swc.bkr.ram.reserved.percent=33```**

* ### swc.bkr.ram.release.rate
```G_INT32```
The Memory release-rate (malloc dependable). \
_default_ **```swc.bkr.ram.release.rate=100```**

* ### swc.bkr.metrics.report.interval
```G_INT32```
The Metrics Reporting Interval in Seconds. Zero(=0) skips metrics update.\
_default_ **```swc.bkr.metrics.report.interval=300```**



***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./swcdbBroker --help;
```

```
./swcdbBroker --help-config;
```

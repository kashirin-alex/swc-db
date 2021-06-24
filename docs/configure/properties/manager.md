---
title: Manager Configuration
---



# The SWC-DB Manager - Configuration Properties



* ### swc.mngr.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. Managers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.mngr.cfg.dyn=```**

* ### swc.mngr.reactors
```INT32```
The number of communication reactor to initialize an io-context for a given acceptor-fd. \
_default_ **```swc.mngr.reactors=8```**

* ### swc.mngr.workers
```INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.mngr.workers=32```**

* ### swc.mngr.handlers
```INT32```
The number of Application handlers. \
_default_ **```swc.mngr.handlers=256```**

* ### swc.mngr.clients.handlers
```INT32```
The number of DB-Clients handlers. \
_default_ **```swc.mngr.clients.handlers=8```**

* ### swc.mngr.metrics.enabled
```BOOL```
Enable or Disable Manager Metrics Monitoring. \
_default_ **```swc.mngr.metrics.enabled=true```**

* ### swc.mngr.metrics.report.broker
```BOOL```
Report Metrics via Broker Client. \
_default_ **```swc.mngr.metrics.report.broker=true```**


***

 > **_The configuration properties applicable for dynamic reloading_**


* ### swc.mngr.comm.encoder
```G_ENUM```
The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.mngr.comm.encoder=ZSTD```**

* ### swc.mngr.metrics.report.interval
```G_INT32```
The Metrics Reporting Interval in Seconds. Zero(=0) skips metrics update.\
_default_ **```swc.mngr.metrics.report.interval=300```**

* ### swc.mngr.role.request.timeout
```G_INT32```
The Timeout in milliseconds of MngrState request. \
_default_ **```swc.mngr.role.request.timeout=60000```**

* ### swc.mngr.role.connection.probes
```G_INT32```
The number of probes to try to establish connection with other Manager. \
_default_ **```swc.mngr.role.connection.probes=3```**

* ### swc.mngr.role.connection.timeout
```G_INT32```
The timeout in milliseconds for establishing connection with other Manager. \
_default_ **```swc.mngr.role.connection.timeout=1000```**

* ### swc.mngr.role.connection.fallback.failures
```G_INT32```
The Manager State becomes OFF after this Number of failures \
_default_ **```swc.mngr.role.connection.fallback.failures=3```**

* ### swc.mngr.role.check.delay.fallback
```G_INT32```
The delay in milliseconds followed by check at fallback.failures reach \
_default_ **```swc.mngr.role.check.delay.fallback=30000```**

* ### swc.mngr.role.check.interval
```G_INT32```
The interval in milliseconds between Managers-Status changes \
_default_ **```swc.mngr.role.check.interval=120000```**

* ### swc.mngr.role.check.delay.updated
```G_INT32```
The Delay in milliseconds on Managers-Status changes \
_default_ **```swc.mngr.role.check.delay.updated=200```**


* ### swc.mngr.ranges.assign.Rgr.remove.failures
```G_UINT16```
The number of failures(establishing connection X probes) after which a Ranger is removed \
_default_ **```swc.mngr.ranges.assign.Rgr.remove.failures=255```**

* ### swc.mngr.ranges.assign.delay.onRangerChange
```G_INT32```
The Delay of Ranges Assignment Check in milliseconds on Ranger state change (on/off) \
_default_ **```swc.mngr.ranges.assign.delay.onRangerChange=30000```**

* ### swc.mngr.ranges.assign.delay.afterColumnsInit
```G_INT32```
The Delay of Ranges Assignment Check in milliseconds follow columns init \
_default_ **```swc.mngr.ranges.assign.delay.afterColumnsInit=30000```**


* ### swc.mngr.ranges.assign.interval.check
```G_INT32```
The Ranges assignment interval in milliseconds between checks \
_default_ **```swc.mngr.ranges.assign.interval.check=60000```**

* ### swc.mngr.ranges.assign.due
```G_INT32```
The total allowed ranges due on Ranger assignment \
_default_ **```swc.mngr.ranges.assign.due=100```**

* ### swc.mngr.column.health.interval.check
```G_INT32```
The Column Health Check Interval in milliseconds \
_default_ **```swc.mngr.column.health.interval.check=300000```**


* ### swc.mngr.column.health.checks
```G_INT32```
The Number of concurrent Column Health Checks \
_default_ **```swc.mngr.column.health.checks=2```**


* ### swc.mngr.schema.replication
```G_INT8```
Save schema under this number of replications (fs-dependent) \
_default_ **```swc.mngr.schema.replication=3```**

* ### swc.mngr.rangers.resource.interval.check
```G_INT32```
The Rangers Resources check interval in milliseconds \
_default_ **```swc.mngr.rangers.resource.interval.check=120000```**

* ### swc.mngr.rangers.range.rebalance.max
```G_INT8```
The Max Ranges to allow for rebalance at once on a Rangers Resources update, Zero is rebalance=Off. \
_default_ **```swc.mngr.rangers.range.rebalance.max=1```**



***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./swcdbManager --help;
```

```
./swcdbManager --help-config;
```

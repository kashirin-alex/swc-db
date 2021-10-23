---
title: Ranger Configuration
---



# The SWC-DB Ranger - Configuration Properties



* ### swc.rgr.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. Rangers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.rgr.cfg.dyn=```**

* ### swc.rgr.reactors
```TYPE_INT32```
The number of communication reactor to initialize an io-context for a given acceptor-fd. \
_default_ **```swc.rgr.reactors=8```**

* ### swc.rgr.workers
```TYPE_INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.rgr.workers=32```**

* ### swc.rgr.handlers
```TYPE_INT32```
The number of Application handlers. \
_default_ **```swc.rgr.handlers=8```**


* ### swc.rgr.maintenance.handlers
```TYPE_INT32```
The number of Maintenance handlers. \
_default_ **```swc.rgr.maintenance.handlers=2```**


* ### swc.rgr.clients.handlers
```TYPE_INT32```
The number of DB-Clients handlers. \
_default_ **```swc.rgr.clients.handlers=8```**

* ### swc.rgr.metrics.enabled
```TYPE_BOOL```
Enable or Disable Ranger Metrics Monitoring. \
_default_ **```swc.rgr.metrics.enabled=true```**

* ### swc.rgr.metrics.report.broker
```TYPE_BOOL```
Report Metrics via Broker Client. \
_default_ **```swc.rgr.metrics.report.broker=true```**


***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.rgr.comm.encoder
```TYPE_ENUM_G```
The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.mngr.comm.encoder=ZSTD```**

* ### swc.rgr.ram.allowed.percent
```TYPE_INT32_G```
The Memory RSS % allowed without freeing/releasing. \
_default_ **```swc.rgr.ram.allowed.percent=33```**

* ### swc.rgr.ram.reserved.percent
```TYPE_INT32_G```
The Memory Total % reserved, threshold of low-memory enter state. \
_default_ **```swc.rgr.ram.reserved.percent=33```**

* ### swc.rgr.ram.release.rate
```TYPE_INT32_G```
The Memory release-rate (malloc dependable). \
_default_ **```swc.rgr.ram.release.rate=100```**

* ### swc.rgr.metrics.report.interval
```TYPE_INT32_G```
The Metrics Reporting Interval in Seconds. Zero(=0) skips metrics update.\
_default_ **```swc.rgr.metrics.report.interval=300```**

* ### swc.rgr.id.validation.interval
```TYPE_INT32_G```
The Validation of Ranger-ID against Manager(RAGERS ROLE). \
_default_ **```swc.rgr.id.validation.interval=120000```**

* ### swc.rgr.compaction.check.interval
```TYPE_INT32_G```
The Interval in milliseconds for Compaction. \
_default_ **```swc.rgr.compaction.check.interval=300000```**

* ### swc.rgr.compaction.read.ahead
```TYPE_UINT8_G```
The Allowed read-ahead scans per Range compaction. \
_default_ **```swc.rgr.compaction.read.ahead=5```**

* ### swc.rgr.compaction.range.max
```TYPE_UINT8_G```
The Max Allowed Ranges at a time for compaction. \
_default_ **```swc.rgr.compaction.range.max=2```**

* ### swc.rgr.compaction.commitlog.max
```TYPE_UINT8_G```
The Max Allowed Commitlog compactions, The compactions count evaluated against total Ranges-Compactions plus the current Commitlog-Compactions. In a case when two Ranges are at compaction for the value of `swc.rgr.compaction.commitlog.max=3`, the allowed commitlog compactions is evaluated to one. \
_default_ **```swc.rgr.compaction.commitlog.max=3```**

* ### swc.rgr.compaction.range.uncompacted.max
```TYPE_INT32_G```
The Max Allowed Ranges with an uncompacted state that is when the CommitLog's modification-time to now is above `swc.rgr.compaction.check.interval` and the CommitLog is not empty. At breach of uncompacted count, compactions are made on Ranges without the need to reach the Range size threshold for compaction. '-1' value disables the checking. \
_default_ **```swc.rgr.compaction.range.uncompacted.max=100```**

* ### swc.rgr.Range.req.update.concurrency
```TYPE_UINT8_G```
The Max Allowed Concurrency a Range for Update Requests processing . \
_default_ **```swc.rgr.Range.req.update.concurrency=1```**

  > ***
  > _Default Schema Values_

* ### swc.rgr.Range.CellStore.count.max
```TYPE_UINT8_G```
The Schema default cellstore-max in range before range-split. \
_default_ **```swc.rgr.Range.CellStore.count.max=10```**


* ### swc.rgr.Range.CellStore.size.max
```TYPE_INT32_G```
The Schema default cellstore-size. \
_default_ **```swc.rgr.Range.CellStore.size.max=1GB```**

* ### swc.rgr.Range.CellStore.replication
```TYPE_UINT8_G```
The Schema default cellstore-replication (fs-dependent). \
_default_ **```swc.rgr.Range.CellStore.replication=3```**

* ### swc.rgr.Range.block.size
```TYPE_INT32_G```
The Schema default block-size. \
_default_ **```swc.rgr.Range.block.size=64MB```**

* ### swc.rgr.Range.block.cells
```TYPE_INT32_G```
The Schema default block-cells. \
_default_ **```swc.rgr.Range.block.cells=100000```**

* ### swc.rgr.Range.block.encoding
```TYPE_ENUM_G```
The Schema default block-encoding NONE/ZSTD/SNAPPY/ZLIB. \
_default_ **```swc.rgr.Range.block.encoding=ZSTD```**

* ### swc.rgr.Range.CommitLog.rollout.ratio
```TYPE_UINT8_G```
The Schema default CommitLog new fragment Rollout Block Ratio. \
_default_ **```swc.rgr.Range.CommitLog.rollout.ratio=3```**

* ### swc.rgr.Range.CommitLog.Compact.cointervaling
```TYPE_UINT8_G```
The minimal sequentially intervaling number of Fragments for CommitLog Compaction to issue a compaction on the selected Fragments. \
_default_ **```swc.rgr.Range.CommitLog.Compact.cointervaling=3```**

* ### swc.rgr.Range.CommitLog.Fragment.preload
```TYPE_UINT8_G```
Preload this number of Fragments of CommitLog at Log Compact and scans(BlockLoader). \
_default_ **```swc.rgr.Range.CommitLog.Fragment.preload=2```**

* ### swc.rgr.Range.compaction.percent
```TYPE_UINT8_G```
The Schema default compact-percent threshold. \
_default_ **```swc.rgr.Range.compaction.percent=33```**



***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./swcdbRanger --help;
```

```
./swcdbRanger --help-config;
```

---
title: ThriftBroker Configuration
---



# The SWC-DB ThriftBroker - Configuration Properties



* ### swc.ThriftBroker.cfg.dyn
```TYPE_STRINGS```
The Dynamic cfg-file to use. ThriftBrokers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.ThriftBroker.cfg.dyn=```**

* ### swc.ThriftBroker.port
```TYPE_UINT16```
The ThriftBroker port. \
_default_ **```swc.ThriftBroker.port=18000```**

* ### swc.ThriftBroker.workers
```TYPE_INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.ThriftBroker.workers=32```**

* ### swc.ThriftBroker.connections.max
```TYPE_INT32```
The Max client Connections allowed, any new connections above the Open-Connections will be dropped and the Max-Total is the number of Endpoints(Thrift-Broker is listening-on) by `swc.ThriftBroker.connections.max`. If open-file-descriptors is above allowed-limit Thrift-Broker will shutdown, unplanned shutdown can be avoided by the max limit. \
_default_ **```swc.ThriftBroker.workers=INT64_MAX```**

* ### swc.ThriftBroker.transport
```TYPE_STRING```
The thrift transport that should be used (framed/). \
_default_ **```swc.ThriftBroker.transport=framed```**

* ### swc.ThriftBroker.timeout
```TYPE_INT32```
The ThriftBroker timeout in milliseconds. \
_default_ **```swc.ThriftBroker.timeout=900000```**

* ### swc.ThriftBroker.clients.handlers
```TYPE_INT32```
The number of SWC-DB clients handlers. \
_default_ **```swc.ThriftBroker.clients.handlers=8```**

* ### swc.ThriftBroker.metrics.enabled
```TYPE_BOOL```
Enable or Disable ThriftBroker Metrics Monitoring. \
_default_ **```swc.ThriftBroker.metrics.enabled=true```**



***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.ThriftBroker.ram.allowed.percent
```TYPE_INT32_G```
The Memory RSS % allowed without freeing/releasing. \
_default_ **```swc.ThriftBroker.ram.allowed.percent=33```**

* ### swc.ThriftBroker.ram.reserved.percent
```TYPE_INT32_G```
The Memory Total % reserved, threshold of low-memory enter state. \
_default_ **```swc.ThriftBroker.ram.reserved.percent=33```**

* ### swc.ThriftBroker.ram.release.rate
```TYPE_INT32_G```
The Memory release-rate (malloc dependable). \
_default_ **```swc.ThriftBroker.ram.release.rate=100```**

* ### swc.ThriftBroker.metrics.report.interval
```TYPE_INT32_G```
The Metrics Reporting Interval in Seconds. Zero(=0) skips metrics update.\
_default_ **```swc.ThriftBroker.metrics.report.interval=300```**


***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./swcdbThriftBroker --help;
```

```
./swcdbThriftBroker --help-config;
```

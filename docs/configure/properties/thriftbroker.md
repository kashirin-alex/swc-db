---
title: ThriftBroker Configuration
---



# The SWC-DB ThriftBroker - Configuration Properties



* ### swc.ThriftBroker.cfg.dyn
```STRINGS```
The Dynamic cfg-file to use. ThriftBrokers will check & reload this cfg-file by swc.cfg.dyn.period \
_default_ **```swc.ThriftBroker.cfg.dyn=```**

* ### swc.ThriftBroker.port
```INT16```
The ThriftBroker port. \
_default_ **```swc.ThriftBroker.port=18000```**

* ### swc.ThriftBroker.workers
```INT32```
The number of workers a comm-reactor initalizes. \
_default_ **```swc.ThriftBroker.workers=32```**

* ### swc.ThriftBroker.connections.max
```INT32```
The Max client Connections allowed, any new connections above the Open-Connections will be dropped and the Max-Total is the number of Endpoints(Thrift-Broker is listening-on) by `swc.ThriftBroker.connections.max`. If open-file-descriptors is above allowed-limit Thrift-Broker will shutdown, unplanned shutdown can be avoided by the max limit. \
_default_ **```swc.ThriftBroker.workers=INT64_MAX```**

* ### swc.ThriftBroker.transport
```STRING```
The thrift transport that should be used (framed/). \
_default_ **```swc.ThriftBroker.transport=framed```**

* ### swc.ThriftBroker.timeout
```INT32```
The ThriftBroker timeout in milliseconds. \
_default_ **```swc.ThriftBroker.timeout=900000```**

* ### swc.ThriftBroker.clients.handlers
```INT32```
The number of SWC-DB clients handlers. \
_default_ **```swc.ThriftBroker.clients.handlers=8```**

* ### swc.ThriftBroker.metrics.enabled
```BOOL```
Enable or Disable ThriftBroker Metrics Monitoring. \
_default_ **```swc.ThriftBroker.metrics.enabled=true```**



***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.ThriftBroker.metrics.report.interval
```G_INT32```
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

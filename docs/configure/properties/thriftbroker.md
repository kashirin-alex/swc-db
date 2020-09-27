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

* ### swc.ThriftBroker.handlers
```INT32```
The number of Application handlers. \
_default_ **```swc.ThriftBroker.handlers=8```**

* ### swc.ThriftBroker.transport
```STRING```
The thrift transport that should be used (framed/). \
_default_ **```swc.ThriftBroker.transport=framed```**


* ### swc.ThriftBroker.timeout
```INT32```
The ThriftBroker timeout in milliseconds. \
_default_ **```swc.ThriftBroker.timeout=900000```**


***

 > **_The configuration properties applicable for dynamic reloading_**

***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./swcdbThriftBroker --help;
```

```
./swcdbThriftBroker --help-config;
```

---
title: Client Library Configuration
---



# The SWC-DB Client Library ```libswcdb``` - Configuration Properties


* ### swc.mngr.port
```INT16```
The Manager default port, value used if not defined in ```swc.mngr.host```. \
_default_ **```swc.mngr.port=15000```**


* ### swc.rgr.port
```INT16```
The Ranger default port, value used if not defined in ```swc.rgr.host```. \
_default_ **```swc.rgr.port=16000```**


***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.cfg.dyn.period
```G_INT32```
The Dynamic cfg-file check interval in milliseconds, The check is Off if value is zero. \
_default_ **```swc.cfg.dyn.period=600000```**


* ### swc.mngr.host
```G_STRINGS```
The Manager Host/s. The value format ```{ROLES}```|```{COLUMNS}```|```{ENDPOINTS}```|```PORT```  delimitted with ```|``` and aligned groupings of Role & Columns is require. The order is part of used configuration defnition to assign host-priority, 1st has initial preference for Active-State.
> **_The Full Format Description_**: \
```{ROLES}``` - In curly-brackets comma-seperated-value, roles options: ```rangers```(manages Rangers), ```schemas``` {manages Schemas}. example ```{rangers,schemas}``` \
```[COLUMNS]``` - In square-brackets, an interval(by Hyphen ```-```) of column-ids, NoValue/Zero on either side is from/to Any. example ```[10-]``` = from 10 to Any. \
```{ENDPOINTS}``` - A ```FQDN``` hostname or a comma-seperated-value of address ```(IPv4 & IPv6)```. example ```192.168.0.1,::2``` \
```PORT``` - optional, If set use this Port with the listed endpoints. \
Multiple ```swc.mngr.host``` require aligment of Role/s and the Column-Intervals.

  _default_ **```swc.mngr.host=```** 

> _**Applicable Combinations of aligned groupings to Role & Columns**_ \
> _Option 1_: to the example - 4-groups, 3-hosts managers of Roles, 6-hosts managers of different 3-column intervals \
  ```swc.mngr.host={rangers,schemas}|host-name-1``` \
  ```swc.mngr.host={rangers,schemas}|host-name-2|15001``` \
  ```swc.mngr.host={rangers,schemas}|host-name-3|15002``` \
  ```swc.mngr.host=[-4]|host-name-4``` \
  ```swc.mngr.host=[-4]|host-name-5``` \
  ```swc.mngr.host=[5-9]|host-name-6``` \
  ```swc.mngr.host=[5-9]|host-name-7``` \
  ```swc.mngr.host=[10-]|host-name-8``` \
  ```swc.mngr.host=[10-]|host-name-9``` \
> _Option 2_: to the example - 3-groups, 4-hosts managers of different Role, 3-hosts managers of all columns \
  ```swc.mngr.host={schemas}|host-name-1``` \
  ```swc.mngr.host={schemas}|host-name-2``` \
  ```swc.mngr.host={rangers}|host-name-3``` \
  ```swc.mngr.host={rangers}|host-name-4``` \
  ```swc.mngr.host=host-name-5``` \
  ```swc.mngr.host=host-name-6``` \
  ```swc.mngr.host=host-name-7``` \
> _Option 3_: to the example - 1-group, 4-hosts managers of all Roles and all columns, one-host is a configuration for a single-manager \
  ```swc.mngr.host=host-name-1``` \
  ```swc.mngr.host=host-name-2``` \
  ```swc.mngr.host=host-name-3``` \
  ```swc.mngr.host=host-name-4```

> _Empty ```swc.mngr.host``` property will result in WARN messages until configurations updated & reloaded._ \
  ```Empty cfg of mngr.host for role=BIT cid=THE_CID```




* ### swc.client.Rgr.connection.timeout
```G_INT32```
The Ranger client connect timeout in milliseconds. \
_default_ **```swc.client.Rgr.connection.timeout=10000```**

* ### swc.client.Rgr.connection.probes
```G_INT32```
The Ranger client connect probes. \
_default_ **```swc.client.Rgr.connection.probes=1```**

* ### swc.client.Rgr.connection.keepalive
```G_INT32```
The Ranger client connection keepalive in milliseconds since last action. \
_default_ **```swc.client.Rgr.connection.keepalive=30000```**

* ### swc.client.Rgr.range.res.expiry
```G_INT32```
The Range(column id + range id) to Ranger Endpoint resolution expiry in milliseconds. \
_default_ **```swc.client.Rgr.range.res.expiry=1800000```**

     
* ### swc.client.Mngr.connection.timeout
```G_INT32```
The Manager client connect timeout in milliseconds. \
_default_ **```swc.client.Mngr.connection.timeout=10000```**

* ### swc.client.Mngr.connection.probes
```G_INT32```
The Manager client connect probes. \
_default_ **```swc.client.Mngr.connection.probes=1```**

* ### swc.client.Mngr.connection.keepalive
```G_INT32```
The Manager client connection keepalive in milliseconds since last action. \
_default_ **```swc.client.Mngr.connection.keepalive=30000```**


* ### swc.client.schema.expiry
```G_INT32```
The Schema cache expiry in milliseconds. \
_default_ **```swc.client.schema.expiry=1800000```**


* ### swc.client.send.buffer
```G_INT32```
The default Client send buffer size in bytes. \
_default_ **```swc.client.send.buffer=8388608```**

* ### swc.client.send.ahead
```G_INT8```
The default Client send number of buffers ahead. \
_default_ **```swc.client.send.ahead=3```**

* ### swc.client.send.timeout
```G_INT32```
The default Client send timeout in milliseconds. \
_default_ **```swc.client.send.timeout=800000```**

* ### swc.client.send.timeout.bytes.ratio
```G_INT32```
The default Client timeout ratio to bytes ```bytes / ratio = milliseconds``` added to ```swc.client.send.timeout```(default Client send timeout). \
_default_ **```swc.client.send.timeout.bytes.ratio=1000```**


* ### swc.client.request.again.delay
```G_INT32```
The Client request again delay in milliseconds. \
_default_ **```swc.client.request.again.delay=500```**


* ### swc.client.recv.buffer
```G_INT32```
The default Client receive buffer size in bytes. \
_default_ **```swc.client.recv.buffer=8388608```**

* ### swc.client.recv.ahead
```G_INT8```
The default Client receive number of buffers ahead. \
_default_ **```swc.client.recv.ahead=3```**

* ### swc.client.recv.timeout
```G_INT32```
The default Client receive timeout in milliseconds. \
_default_ **```swc.client.recv.timeout=800000```**

***


 > _**extended/updated/version information available, for the Programs using SWC-DB Client Library with '--help' and '--help-config' arg**_

```
./aProgram --help;
```

```
./aProgram --help-config;
```

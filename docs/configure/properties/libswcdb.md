---
title: Client Library Configuration
---



# The SWC-DB Client Library ```libswcdb``` - Configuration Properties


* ### swc.mngr.port
```TYPE_UINT16```
The Manager default port, value used if not defined in ```swc.mngr.host```. \
_default_ **```swc.mngr.port=15000```**


* ### swc.rgr.port
```TYPE_UINT16```
The Ranger default port, value used if not defined in ```swc.rgr.host```. \
_default_ **```swc.rgr.port=16000```**


* ### swc.bkr.port
```TYPE_UINT16```
The Broker default port, value used if not defined in ```swc.bkr.host```. \
_default_ **```swc.bkr.port=17000```**


***

 > **_The configuration properties applicable for dynamic reloading_**

* ### swc.cfg.dyn.period
```TYPE_INT32_G```
The Dynamic cfg-file check interval in milliseconds, The check is Off if value is zero. \
_default_ **```swc.cfg.dyn.period=600000```**


* ### swc.mngr.host
```TYPE_STRINGS_G```
The Manager Host/s. The value format ```{ROLES}```|```{COLUMNS}```|```{ENDPOINTS}```|```PORT```  delimitted with ```|``` and aligned groupings of Role & Columns is require. The order is part of used configuration defnition to assign host-priority, 1st has initial preference for Active-State.
> **_The Full Format Description_**: \
```{ROLES}``` - In curly-brackets comma-seperated-value, roles options: ```rangers```(manages Rangers), ```schemas``` {manages Schemas}. example ```{rangers,schemas}``` \
```[COLUMNS]``` - In square-brackets, an interval(by Hyphen ```-```) of column-ids, From and NoValue/Zero is to Any. example ```[10-]``` = from 10 to Any. \
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
  ```swc.mngr.host=[1-]|host-name-5``` \
  ```swc.mngr.host=[1-]|host-name-6``` \
  ```swc.mngr.host=[1-]|host-name-7``` \
> _Option 3_: to the example - 1-group, 4-hosts managers of all Roles and all columns, one-host is a configuration for a single-manager \
  ```swc.mngr.host=host-name-1``` \
  ```swc.mngr.host=host-name-2``` \
  ```swc.mngr.host=host-name-3``` \
  ```swc.mngr.host=host-name-4```

> _Empty ```swc.mngr.host``` property will result in WARN messages until configurations updated & reloaded._ \
  ```Empty cfg of mngr.host for role=BIT cid=THE_CID```




* ### swc.bkr.host
```TYPE_STRINGS_G```
The Broker Host/s. Client will try to establish connection and proceed with the Broker hosts available by the order specified. \
Value in format ```{ENDPOINTS}```|```PORT```, if PORT not specified ```swc.bkr.port``` is applied, ENDPOINTS a FQDN or comma-separated IPs as ```IPv4,IPv4,IPv6```.\
_default_ **```swc.bkr.host=```**

* ### swc.client.Bkr.connection.timeout
```TYPE_INT32_G```
The Broker client connect timeout in milliseconds. \
_default_ **```swc.client.Bkr.connection.timeout=10000```**

* ### swc.client.Bkr.connection.probes
```TYPE_UINT16_G```
The Broker client connect probes. \
_default_ **```swc.client.Bkr.connection.probes=1```**

* ### swc.client.Bkr.connection.keepalive
```TYPE_INT32_G```
The Broker client connection keepalive in milliseconds since last action. \
_default_ **```swc.client.Bkr.connection.keepalive=30000```**

* ### swc.client.Bkr.comm.encoder
```TYPE_ENUM_G```
The encoding to use in communication with Broker, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.client.Bkr.comm.encoder=ZSTD```**


* ### swc.client.Rgr.connection.timeout
```TYPE_INT32_G```
The Ranger client connect timeout in milliseconds. \
_default_ **```swc.client.Rgr.connection.timeout=10000```**

* ### swc.client.Rgr.connection.probes
```TYPE_UINT16_G```
The Ranger client connect probes. \
_default_ **```swc.client.Rgr.connection.probes=1```**

* ### swc.client.Rgr.connection.keepalive
```TYPE_INT32_G```
The Ranger client connection keepalive in milliseconds since last action. \
_default_ **```swc.client.Rgr.connection.keepalive=30000```**

* ### swc.client.Rgr.comm.encoder
```TYPE_ENUM_G```
The encoding to use in communication with Ranger, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.client.Rgr.comm.encoder=ZSTD```**

* ### swc.client.Rgr.range.res.expiry
```TYPE_INT32_G```
The Range(column id + range id) to Ranger Endpoint resolution expiry in milliseconds. \
_default_ **```swc.client.Rgr.range.res.expiry=1800000```**


* ### swc.client.Mngr.connection.timeout
```TYPE_INT32_G```
The Manager client connect timeout in milliseconds. \
_default_ **```swc.client.Mngr.connection.timeout=10000```**

* ### swc.client.Mngr.connection.probes
```TYPE_UINT16_G```
The Manager client connect probes. \
_default_ **```swc.client.Mngr.connection.probes=1```**

* ### swc.client.Mngr.connection.keepalive
```TYPE_INT32_G```
The Manager client connection keepalive in milliseconds since last action. \
_default_ **```swc.client.Mngr.connection.keepalive=30000```**

* ### swc.client.Mngr.comm.encoder
```TYPE_ENUM_G```
The encoding to use in communication with Manager, options PLAIN/ZSTD/SNAPPY/ZLIB.
> If address of local & remote is the same, the Encoder is set to PLAIN.

  _default_ **```swc.client.Mngr.comm.encoder=ZSTD```**

* ### swc.client.Mngr.range.master.expiry
```TYPE_INT32_G```
The Located Master-Range and Ranger cache expiry in milliseconds. \
_default_ **```swc.client.Mngr.range.master.expiry=1800000```**


* ### swc.client.schema.expiry
```TYPE_INT32_G```
The Schema cache expiry in milliseconds. \
_default_ **```swc.client.schema.expiry=1800000```**


* ### swc.client.send.buffer
```TYPE_INT32_G```
The default Client send buffer size in bytes. \
_default_ **```swc.client.send.buffer=8388608```**

* ### swc.client.send.ahead
```TYPE_UINT8_G```
The default Client send number of buffers ahead. \
_default_ **```swc.client.send.ahead=3```**

* ### swc.client.send.timeout
```TYPE_INT32_G```
The default Client send timeout in milliseconds. \
_default_ **```swc.client.send.timeout=800000```**

* ### swc.client.send.timeout.bytes.ratio
```TYPE_INT32_G```
The default Client timeout ratio to bytes ```bytes / ratio = milliseconds``` added to ```swc.client.send.timeout```(default Client send timeout). \
_default_ **```swc.client.send.timeout.bytes.ratio=1000```**


* ### swc.client.request.again.delay
```TYPE_INT32_G```
The Client request again delay in milliseconds. \
_default_ **```swc.client.request.again.delay=500```**


* ### swc.client.recv.buffer
```TYPE_INT32_G```
The default Client receive buffer size in bytes. \
_default_ **```swc.client.recv.buffer=8388608```**

* ### swc.client.recv.ahead
```TYPE_UINT8_G```
The default Client receive number of buffers ahead. \
_default_ **```swc.client.recv.ahead=3```**

* ### swc.client.recv.timeout
```TYPE_INT32_G```
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

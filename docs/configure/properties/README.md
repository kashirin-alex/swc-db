---
title: The Properties
sort: 2
---




# The Configuration Properties
  **Any configuration property, if it is shown for ```./program``` with ```--help``` or ```--help-config```, can be a Command Line argument by using it with a prefix ```--``` while Command Line argument is not a cfg-file property** .
  
  **_Configuration Properties by Program & Library:_**
  * [The SWC-DB Configurations Library ```libswcdb_core_config```](#the-initial-libswcdb_core_config---configuration-properties)
  * [The SWC-DB Communications Library ```libswcdb_core_comm```](#the-initial-communications-libswcdb_core_comm---configuration-properties)
  * [The SWC-DB Manager Program ```swcdbManager```](manager.html)
  * [The SWC-DB Ranger Program ```swcdbRanger```](ranger.html)
  * [The SWC-DB Filesystem Broker Program ```swcdbFsBroker```](fsbroker.html)
  * [The SWC-DB Thrift Broker Program ```swcdbThriftBroker```](thriftbroker.html)
  * [The SWC-DB Filesystem Library ```libswcdb_fs```](libswcdb_fs/)
  * [The SWC-DB Client Library ```libswcdb```](libswcdb.html)


***



## The Initial libswcdb_core_config - Configuration Properties
These properties are the initial settings that applied to any Program using the class SWC::Config::Settings.


> ***
> **_Only Command Line argument properties_**

* ### --help / -h
```BOOL```
Show the help contents for executing the Program.

* ### --help-config
```BOOL```
Show the help contents with the Program's configuration properties.

* ### --version / -v
```BOOL```
Show version of the Program.


* ### --debug
```BOOL```
a Shortcut to --swc.logging.level=debug . \
_default_ **```False```**

* ### --quiet
```BOOL```
Minimal output, logging remains by swc.logging.level . \
_default_ **```False```**

* ### --daemon
```BOOL```
Start process in background mode, if Program supports it . \
_default_ **```False```**


> ***
> **_File Configuration Properties_**

* ### swc.cfg.path
```STRING```
The path to location of configuration files. \
_default_ **```swc.cfg.path=SWC_INSTALL_PREIFX/etc/swcdb/```**

* ### swc.cfg
```STRING```
The main configuration file, everything begins with this file . \
_default_ **```swc.cfg=swc.cfg```**

* ### swc.cfg.dyn
```STRINGS```
The main dynamic configuration file. \
_default_ **```swc.cfg.dyn=```**


* ### swc.logging.path
```STRINGS```
The path of log directories and files, ```YYYY/MM/DD``` sub-folders are create on this path. \
_default_ **```swc.logging.path=SWC_INSTALL_PREIFX/var/log/swcdb/```**


***

> **_The configuration properties applicable for dynamic reloading_**

* ### swc.logging.level
```G_ENUM```
The Logging Level: debug|info|notice|warn|error|crit|alert|fatal. \
_default_ **```swc.logging.level=INFO```**



***



## The Initial Communications libswcdb_core_comm - Configuration Properties
These properties are the initial communication settings that applied to any Program using the Settings::init_comm_options.


> ***
> **_Only Command Line argument properties_**

* ### --addr
```STRINGS```
The address(IP-port) to listen on else resolved by hostname with swc.ServiceName.port. \
_Multiple argument is allowed ```./program --addr=ADDR-1 --addr=ADDR-2```_ \
_default_ **```--addr=```**

* ### --host
```STRING```
The host:port to resolve, listens on resolved IPv4 + IPv6, swc.ServiceName.port applied if port not specified. \
_default_ **```--host=```**



> ***
> **_File Configuration Properties_**


* ### swc.comm.network.priority
```STRINGS```
The Network Priority Access, By available endpoints of a server, try to establish connection in this priority of a network.
> _recommended priority order:_ \
  ```swc.comm.network.priority = 127.0.0.0/8``` \
  ```swc.comm.network.priority = ::1/128``` \
  ```swc.comm.network.priority = 192.168.0.0/16``` \
  ```swc.comm.network.priority = 1::/64``` \
  ```swc.comm.network.priority = 172.16.0.0/12``` \
  ```swc.comm.network.priority = 10.0.0.0/8``` \
  ```swc.comm.network.priority = fc00::/7```

  _default_ **```swc.comm.network.priority```**

* ### swc.comm.ssl
```BOOL```
whether to use SSL in communications layer. \
_default_ **```swc.comm.ssl=false```**

* ### swc.comm.ssl.secure.network
```STRINGS```
The Networks that do not require SSL.
> _recommended:_ \
  ```swc.comm.ssl.secure.network = 127.0.0.0/8``` \
  ```swc.comm.ssl.secure.network = ::1/128```

  _default_ **```swc.comm.ssl.secure.network=```**

* ### swc.comm.ssl.ciphers
```STRING```
The Ciphers to use, in format of openSSL for one string (delimitted with a colon ```:```). \
_default_ **```swc.comm.ssl.ciphers=```**

* ### swc.comm.ssl.subject_name
```STRING```
The Certificate or Cluster's domain-name, if set SRV-CRT is verified. \
_default_ **```swc.comm.ssl.subject_name=```**

* ### swc.comm.ssl.crt
```STRING```
The Cluster Certificate file. Filename without slash/dot applied on swc.cfg.path . \
_default_ **```swc.comm.ssl.crt=cluster.crt```**

* ### swc.comm.ssl.key
```STRING```
The Server Private-Key file. Filename without slash/dot applied on swc.cfg.path . \
_default_ **```swc.comm.ssl.key=cluster.key```**


* ### swc.comm.ssl.ca
```STRING```
The CA, used if set. Filename without slash/dot applied on swc.cfg.path . \
_default_ **```swc.comm.ssl.ca=```**

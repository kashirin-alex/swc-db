---
title: Cluster Managing Script
---



# Cluster Managing Script ```swcdb_cluster``` - Configuration Properties
These configuration properties are for sbin/swcdb_cluster a python script based on [py-fabric library](https://www.fabfile.org/).


* ### swc.cluster.role.delay.start
```INT64```
The Delay in seconds to wait between roles executions. \
_default_ **```swc.cluster.role.delay.start=2```**


* ### swc.cluster.rgr.host
```STRINGS```
A Ranger host, value in format ```default|hostname|port``` OR ```default|ip,ip,ip|port```. The hostname/ips need to be accessible from the source-host. 
> without ```|port``` - no port applied and Program uses the default port \
> with ```default|``` - hostname/address used for ssh, no host applied to the Program and Program will use the default addresses.

  _default_ **```swc.cluster.rgr.host=```**


* ### swc.cluster.fsbroker.host
```STRINGS```
A FsBroker host, value in format ```default|hostname|port``` OR ```default|ip,ip,ip|port```. The hostname/ips need to be accessible from the source-host. 
> without ```|port``` - no port applied and Program uses the default port \
> with ```default|``` - hostname/address used for ssh, no host applied to the Program and Program will use the default addresses.

  _default_ **```swc.cluster.fsbroker.host=```**



* ### swc.cluster.thriftbroker.host
```STRINGS```
A ThriftBroker host, value in format ```default|hostname|port``` OR ```default|ip,ip,ip|port```. The hostname/ips need to be accessible from the source-host. 
> without ```|port``` - no port applied and Program uses the default port \
> with ```default|``` - hostname/address used for ssh, no host applied to the Program and Program will use the default addresses.

  _default_ **```swc.cluster.thriftbroker.host=```**



***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./sbin/swcdb_cluster --help;
```


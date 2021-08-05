---
title: Cluster Managing Script
---



# The SWC-DB Cluster Managing Script ```swcdb_cluster``` - Configuration Properties
These configuration properties are for sbin/swcdb_cluster a python script based on [py-fabric library](https://www.fabfile.org/).


* ### swc.install.archive
```STRING```
The Full-Path or Uri to the SWC-DB `tar` archive package. An archive package from [available SWC-DB releases]({{ site.baseurl }}/install/getting_swcdb/#available-for-download) is suitable for the type of archive required.
> The source-host SWC-DB installation files are copied if the property is not set.

  _default_ **```swc.install.archive=https://github.com/kashirin-alex/swc-db/releases/download/v0.5.4/swcdb-0.5.4.debug.amd64.tar.xz```**


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


* ### swc.cluster.bkr.host
```STRINGS```
A Broker host, value in format ```default|hostname|port``` OR ```default|ip,ip,ip|port```. The hostname/ips need to be accessible from the source-host.
> without ```|port``` - no port applied and Program uses the default port \
> with ```default|``` - hostname/address used for ssh, no host applied to the Program and Program will use the default addresses.

  _default_ **```swc.cluster.bkr.host=```**



* ### swc.cluster.thriftbroker.host
```STRINGS```
A ThriftBroker host, value in format ```default|hostname|port``` OR ```default|ip,ip,ip|port```. The hostname/ips need to be accessible from the source-host.
> without ```|port``` - no port applied and Program uses the default port \
> with ```default|``` - hostname/address used for ssh, no host applied to the Program and Program will use ```localhost``` address.

  _default_ **```swc.cluster.thriftbroker.host=```**



***

 > _**extended/updated/version information available with '--help' and '--help-config' arg**_

```
./sbin/swcdb_cluster --help;
```


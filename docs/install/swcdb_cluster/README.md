---
title: Setting up swcdb_cluster
sort: 3
---

# Setting up swcdb_cluster

`sbin/swcdb_cluster` is a Fabric-based Python utility that starts, stops, and deploys SWC-DB daemons across localhost (pseudomode) or remote hosts (distributed). Source: `src/py/sbin/swcdb_cluster`.


### **swcdb_cluster uses fabric python-package**
Install the `fabric` python-package:
```bash
python3 -m pip install setuptools fabric;
```


### **swcdb_cluster requires password-less ssh authentication**
Create a RSA-Key:
```bash
ssh-keygen -t rsa;
```

To pre-authorize known localhosts-addresses in-order to skip manual host key approval:
```bash
cat /root/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys;
ssh-keyscan -t rsa localhost,ip6-localhost,localhost.localdomain,::1,::,127.0.0.1 >> ~/.ssh/known_hosts;
```

#### Known Issues
  * On execution ```paramiko.ssh_exception.SSHException``` exception is raised with a message "key cannot be used for signing" or an "Unknown/Unsupported" Key or not a valid RSA private key file.
  > The reason can be badly-configured or broken packages distributed by the OS, an immediate possible solution is to uninstall the packages installed by the OS package manager (on Ubuntu with `apt-get remove python-paramiko python-fabric`, on Archlinux `pacman -R python-paramiko python-fabric`) and to install the packages available with pip.



***



## Hosts and config

Role hosts are read from `etc/swcdb/` (including `swc.cluster.cfg` / dynamic cfg):

* Managers: `swc.mngr.host`
* Rangers / FsBrokers / Brokers / ThriftBrokers: `swc.cluster.rgr.host`, `swc.cluster.fsbroker.host`, `swc.cluster.bkr.host`, `swc.cluster.thriftbroker.host`

Host value format (see [Cluster Managing Script properties]({{ site.baseurl }}/configure/properties/swcdb_cluster.md)):

```text
default|hostname|port
default|ip,ip,ip|port
```

For a single-host pseudomode, localhost entries are enough. For multi-host, list each role’s SSH-reachable hosts in those properties.



## Common commands

From the install prefix (for example `/opt/swcdb`):

```bash
cd /opt/swcdb
sbin/swcdb_cluster --help
```

| Subcommand | Purpose |
|------------|---------|
| `start` | Start configured daemons (and wait until ready) |
| `stop` | Graceful stop |
| `kill` / `kill-*` | Force-stop roles |
| `deploy` | Install/copy packages to remote hosts (`swc.install.archive`) |
| `push-config` | Push config files to cluster hosts |
| `shell` | Interactive Fabric shell |

Examples:

```bash
sbin/swcdb_cluster start
sbin/swcdb_cluster stop
```

Distributed run notes: [Running Distributed]({{ site.baseurl }}/run/distributed/). Property reference: [swcdb_cluster properties]({{ site.baseurl }}/configure/properties/swcdb_cluster.md).

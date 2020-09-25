---
title: Distributed Cluster
sort: 2
---


# Running SWC-DB in a Distributed Cluster

_Edit the necessary configuration in etc/swcdb/*.cfg [as by instructions]({{ site.baseurl }}/configure/)_

```
cd /opt/swcdb;          # SWCDB_INSTALL_PATH
cd sbin/;
```

## the Utility 'swcdb_cluster '

  **swcdb_cluster needs to set [as by instructions]({{ site.baseurl }}/install/swcdb_cluster/)**
  
  _List the available commdand of swcdb_cluster_

```
./swcdb_cluster --help;
```

```text 
Usage: swcdb_cluster [--core-opts] <subcommand> [--subcommand-opts] ...

Subcommands:

  deploy
  push-config
  shell
  start
  start-fsbrokers
  start-managers
  start-rangers
  start-thriftbrokers
  stop
  stop-fsbrokers
  stop-managers
  stop-rangers
  stop-thriftbrokers
  wait-ready

```

### To deploy on the configured cluster and start
```
./swcdb_cluster deploy; 
```
> The **'start'** command will wait for **'wait-ready'** command of the cluster ready state with results output of the progress.
```
./swcdb_cluster start;
```

### To stop the configured cluster
> The 'stop' command will wait for a graceful shutdown
```
./swcdb_cluster stop;
```

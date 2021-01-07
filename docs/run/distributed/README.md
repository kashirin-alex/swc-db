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
  kill
  kill-fsbrokers
  kill-managers
  kill-rangers
  kill-thriftbrokers
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


### To deploy SWC-DB on the configured cluster
In order to proceed with the `deploy` command, you should have configured by the [configurations instructions of the `swcdb_cluster`]({{ site.baseurl }}/configure/properties/swcdb_cluster.html).
> The deploy command will:
  * if from tar archive:
    * download the Archive, if the [`the swc.install.archive`]({{ site.baseurl }}/configure/properties/swcdb_cluster.html#swcinstallarchive) is set to URI.
    * copy the tar and extract the tar archive on all the hosts of the SWC-DB cluster
    * push-config from source-host etc/swcdb/ to all the hosts of the SWC-DB cluster
  * if from source-host installation:
    * copy all the files of SWC-DB install-path to all the hosts of the SWC-DB cluster

```
./swcdb_cluster deploy; 
```

### To start the configured SWC-DB cluster
> The **'start'** command will wait for **'wait-ready'** command of the cluster ready state with results output of the progress.
```
./swcdb_cluster start;
```

### To stop the SWC-DB cluster
> The 'stop' command will wait for a graceful shutdown
```
./swcdb_cluster stop;
```

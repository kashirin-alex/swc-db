---
title: Pseudomode
sort: 1
---


# Running SWC-DB in a Pseudomode

A local all-in-one cluster on a single host (typically `/opt/swcdb` after install).


## Minimal configuration checklist

After installing a package into `/opt/swcdb`, the shipped files under `etc/swcdb/` are enough for a first local run in most cases:

1. Confirm `etc/swcdb/` exists (from the package or `make install`).
2. Defaults usually keep `swc.fs=local` and localhost ports — no edits required for a smoke test.
3. Create the log directory (or set `swc.logging.path` in config):

```
cd /opt/swcdb;          # SWCDB_INSTALL_PATH
mkdir -p var/log/swcdb; # re-config "swc.logging.path" for other path
```

For non-default layouts, FS backends, or multi-homed hosts, see [Configuring]({{ site.baseurl }}/configure/) and [The Config Files]({{ site.baseurl }}/configure/the_config_files/).



## STARTING-UP OPTIONS


* ### start with sbin/swcdb_cluster
_**sbin/swcdb_cluster** can start **only on localhost** as if it is a distributed-cluster, if swcdb_cluster is set [as by instructions]({{ site.baseurl }}/install/swcdb_cluster/)_.
```
sbin/swcdb_cluster start;
```


* ### start each server independently
```
cd bin; # continue to work from 'bin' folder
```

  1. #### start swcdbFsBroker
```
./swcdbFsBroker --daemon;
```
  2. #### start swcdbManager
```
./swcdbManager --debug --host=localhost --daemon;
```
  3. #### start swcdbRanger
```
./swcdbRanger --daemon;
```
  4. #### start swcdbBroker
```
./swcdbBroker --host=localhost --daemon;
```
  5. #### start swcdbThriftBroker
```
./swcdbThriftBroker --host=localhost --daemon;
```

  * #### Additional Information
    * Help information available with ```./swcdbProgramName --help;```
    * without ```--daemon``` argument the Program will output to stdout/screen instead of logging to file.




## SHUTTING-DOWN OPTIONS

* ### stop with sbin/swcdb_cluster
```
sbin/swcdb_cluster stop;
```

* ### stop with SIGINT
By the runtime order dependency for each PID of a server/program/swcdbProgramName:
  1. swcdbThriftBroker
  2. swcdbBroker
  3. swcdbRanger
  4. swcdbManager
  5. swcdbFsBroker

  ```bash
ps aux | grep swcdbProgramName;
kill PID;
```
  > ```kill -9 PID;``` will result in ungraceful shutdown (not committed data and current transactions will be lost)

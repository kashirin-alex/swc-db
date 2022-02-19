---
title: Pseudomode
sort: 1
---


# Running SWC-DB in a Pseudomode

_Edit the necessary configuration in etc/swcdb/*.cfg [as by instructions]({{ site.baseurl }}/configure/)_

```
cd /opt/swcdb;          # SWCDB_INSTALL_PATH
mkdir -p var/log/swcdb; # re-config "swc.logging.path" for other path
```



## STARING-UP OPTIONS


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

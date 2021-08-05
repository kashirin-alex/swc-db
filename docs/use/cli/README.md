---
title: Command Line Interfaces
sort: 1
---



# Using the SWC-DB Command Line Interfaces (CLI)

The SWC-DB CLI/Shell is available in the utillity program `bin/swcdb` and the default Shell is the ```SWC-DB(client)>``` interface.


**The Command Line Interfaces and argument-values available to work with are:**

| Argument Token                     | Shell                                                           |  Command Line Interface   |
| ---                                | ---                                                             | ---                       |
|  _without an argument_             | **[Work with the DB-Client](db_client/)**                       | ```SWC-DB(client)>```     |
| ```--manager```     / ```-mngr```  | **[Work with a Manager](manager/)**                             | ```SWC-DB(mngr)>```       |
| ```--ranger```      / ```-rgr```   | **[Work with a Ranger](ranger/)**                               | ```SWC-DB(rgr)>```        |
| ```--filesystem```  / ```-fs```    | **[Work with a FileSystem](filesystem/)** type by swc.fs='TYPE' | ```SWC-DB(fs-TYPE)>```    |
| ```--statistics```  / ```-stats``` | **[Work with Statistics monitor](statistics/)**                 | ```SWC-DB(statistics)>``` |


To use the specific CLI apply the argument-value-token at ```bin/swcdb --ARG;``` execution. It is possible to switch between the available CLIs at the shell interface with the command ```switch to CLI;```.


* For additional help information with running, use: ```bin/swcdb --help;``` and ```bin/swcdb --help-config;```
* For help with available shell-commands and help with the commands execution, use: ```SWC-DB('cli-name')> help;``` in the selected CLI.


***


```bash
./swcdb --help;
```
```bash
SWC-DB(utils) Usage: swcdb 'command' [options]

Options:
  --command -[cmd]                        Command to execute shell|status|report|custom (1st arg token)         shell
  --daemon                                Start process in background mode                                      true
  --debug                                 Shortcut to --swc.logging.level debug                                 false
  --filesystem -[fs]                      Work with FileSystem type by swc.fs='type'                            true
  --help -[h]                             Show this help message and exit                                       true
  --help-config                           Show help message for config properties                               true
  --lib                                   Utility-Library of the custom command
  --lib-path                              Path to utilities libraries                                           /opt/swcdb/lib/
  --manager -[mngr]                       Work with Manager                                                     true
  --quiet                                 Negate verbose                                                        false
  --ranger -[rgr]                         Work with Ranger                                                      true
  --statistics -[stats, monit]            Work with Statistics monitor                                          true
  --swc.bkr.host                          Broker Host: "(hostname or ips-csv)|port"                             []
  --swc.bkr.port                          Broker port                                                           17000
  --swc.cfg                               Main configuration file                                               swc.cfg
  --swc.cfg.dyn                           Main dynamic configuration file                                       []
  --swc.cfg.dyn.period                    Dynamic cfg-file check interval in ms, zero without                   600000
  --swc.cfg.path                          Path of configuration files                                           /opt/swcdb/etc/swcdb/
  --swc.client.Bkr.comm.encoder           The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB  ZSTD  # (4)
  --swc.client.Bkr.connection.keepalive   Broker client connection keepalive for ms since last action           30000
  --swc.client.Bkr.connection.probes      Broker client connect probes                                          1
  --swc.client.Bkr.connection.timeout     Broker client connect timeout                                         10000
  --swc.client.Mngr.comm.encoder          The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB  ZSTD  # (4)
  --swc.client.Mngr.connection.keepalive  Manager client connection keepalive for ms since last action          30000
  --swc.client.Mngr.connection.probes     Manager client connect probes                                         1
  --swc.client.Mngr.connection.timeout    Manager client connect timeout                                        10000
  --swc.client.Mngr.range.master.expiry   Cached Master Range expiry in ms                                      1800000
  --swc.client.Rgr.comm.encoder           The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB  ZSTD  # (4)
  --swc.client.Rgr.connection.keepalive   Ranger client connection keepalive for ms since last action           30000
  --swc.client.Rgr.connection.probes      Ranger client connect probes                                          1
  --swc.client.Rgr.connection.timeout     Ranger client connect timeout                                         10000
  --swc.client.Rgr.range.res.expiry       Range Ranger resolution expiry in ms                                  1800000
  --swc.client.recv.ahead                 Client receive number of buffers ahead                                3
  --swc.client.recv.buffer                Client receive buffer size in bytes                                   8388608
  --swc.client.recv.timeout               Client receive timeout in ms                                          800000
  --swc.client.request.again.delay        Client request again delay size in ms                                 500
  --swc.client.schema.expiry              Schemas expiry in ms                                                  1800000
  --swc.client.send.ahead                 Client send number of buffers ahead                                   3
  --swc.client.send.buffer                Client send buffer size in bytes                                      8388608
  --swc.client.send.timeout               Client send timeout in ms                                             800000
  --swc.client.send.timeout.bytes.ratio   Timeout ratio to bytes, bytes/ratio=ms added to send timeout          1000
  --swc.logging.level -[l]                Logging level: debug|info|notice|warn|error|crit|alert|fatal          INFO  # (6)
  --swc.logging.path                      Path of log files                                                     /opt/swcdb/var/log/swcdb/
  --swc.mngr.host                         Manager Host: "[cols range]|(hostname or ips-csv)|port"               []
  --swc.mngr.port                         Manager default port if not defined in swc.mngr.host                  15000
  --swc.rgr.port                          Ranger port                                                           16000
  --verbose                               Show more verbose output                                              false
  --version -[v]                          Show version information and exit                                     true
  --with-broker                           Query applicable requests with Broker                                 false

```



```bash
./swcdb --help-config
```
```bash

  --addr                         IP-port, addr to listen on else resolved(hostname):swc.ServiceName.port            []
  --host                         host:port to listen on IPv4+6, swc.ServiceName.port applied if port not specified
  --swc.comm.network.priority    Network Priority Access                                                            []
  --swc.comm.ssl                 Use SSL in comm layer                                                              false
  --swc.comm.ssl.ca              CA, used if set
  --swc.comm.ssl.ciphers         Ciphers to use
  --swc.comm.ssl.crt             Cluster Certificate file                                                           cluster.crt
  --swc.comm.ssl.key             Server Private-Key file                                                            server.key
  --swc.comm.ssl.secure.network  Networks that do not require SSL                                                   []
  --swc.comm.ssl.subject_name    CRT/Cluster domain-name, if set SRV-CRT is verified
  --swc.fs                       main FileSystem: local|hadoop|hadoop_jvm|ceph|broker|custom
  --swc.fs.broker.cfg            Specific cfg-file for FS-broker
  --swc.fs.broker.underlying     as main FileSystem, without 'broker': local|hadoop|ceph|custom
  --swc.fs.ceph.cfg              Specific cfg-file for FS-ceph
  --swc.fs.custom.cfg            Specific cfg-file for FS-custom
  --swc.fs.hadoop.cfg            Specific cfg-file for FS-hadoop
  --swc.fs.hadoop_jvm.cfg        Specific cfg-file for FS-hadoop_jvm
  --swc.fs.lib.broker            FS-broker Lib-path based on fs/FileSystem.h
  --swc.fs.lib.ceph              FS-ceph Lib-path based on fs/FileSystem.h
  --swc.fs.lib.custom            FS-custom Lib-path based on fs/FileSystem.h
  --swc.fs.lib.hadoop            FS-hadoop Lib-path based on fs/FileSystem.h
  --swc.fs.lib.hadoop_jvm        FS-hadoop-JVM Lib-path based on fs/FileSystem.h
  --swc.fs.lib.local             FS-local Lib-path based on fs/FileSystem.h
  --swc.fs.local.cfg             Specific cfg-file for FS-local
  --swc.fs.path.data             SWC-DB data-path, within the FS(specific) base-path                                swcdb/

```
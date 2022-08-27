---
title: Load Generator
sort: 11
---


# Using SWC-DB load generator

```bash
cd /opt/swcdb;          # if SWCDB_INSTALL_PATH not on PATH
```

###### Enter Help

```bash
./swcdb_load_generator --help;
```

```text
# SWC-DB (c) Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# SWC-DB is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version GPLv3.
#
# SWC-DB is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.
# If not, see <https://github.com/kashirin-alex/swc-db/blob/v0.5.5/LICENSE>.

SWC-DB(load_generator) Usage: swcdb_load_generator [options]

Options:
  --daemon                                Start process in background mode                                             true
  --debug                                 Shortcut to --swc.logging.level debug                                        false
  --gen-blk-cells                         Schema blk-cells                                                             0
  --gen-blk-encoding                      Schema blk-encoding NONE|ZSTD|SNAPPY|ZLIB                                    DEFAULT  # (0)
  --gen-blk-size                          Schema blk-size                                                              0
  --gen-cell-a-time                       Write one cell at a time                                                     false
  --gen-cell-encoding                     Cell's Value encoding ZSTD|SNAPPY|ZLIB                                       PLAIN  # (1)
  --gen-cell-versions                     cell key versions                                                            1
  --gen-cells                             number of cells, total=cells*versions*(is SINGLE?1:fractions*onLevel)        1000
  --gen-cells-on-level                    number of cells, on fraction level                                           1
  --gen-col-name                          Gen. load column name, joins with colm-number                                load_generator-
  --gen-col-number                        Number of columns to generate                                                1
  --gen-col-seq                           Schema col-seq FC_+|LEXIC|VOLUME                                             LEXIC  # (1)
  --gen-col-type                          Schema col-type PLAIN|COUNTER_I{64,32,16,8}|SERIAL                           PLAIN  # (1)
  --gen-compaction-percent                Compaction threshold in % applied over size of either by cellstore or block  0
  --gen-cs-count                          Schema cs-count                                                              0
  --gen-cs-replication                    Schema cs-replication                                                        0
  --gen-cs-size                           Schema cs-size                                                               0
  --gen-delete                            Delete generated data                                                        false
  --gen-delete-column                     Delete Column after                                                          false
  --gen-distrib                           Distribution SEQUENTIAL|STEPPING|UNIFORM                                     SEQUENTIAL  # (0)
  --gen-distrib-course                    Fractions distrib Course STEP|R_STEP|SINGLE|R_SINGLE|LEVELS|R_LEVELS         STEP  # (0)
  --gen-distrib-seed                      Use this seed/step for Distribution injection                                1
  --gen-fraction-size                     fraction size in bytes at least                                              10
  --gen-fractions                         Number of Fractions per cell key                                             10
  --gen-handlers                          Number of client Handlers                                                    8
  --gen-insert                            Generate new data                                                            true
  --gen-log-compact-cointervaling         CommitLog minimal cointervaling Fragments for compaction                     0
  --gen-log-preload                       Number of CommitLog Fragments to preload                                     0
  --gen-log-rollout                       CommitLog rollout block ratio                                                0
  --gen-progress                          display progress every N cells or 0 for quiet                                100000
  --gen-select                            Select generated data                                                        false
  --gen-select-empty                      Expect empty select results                                                  false
  --gen-value-size                        cell value in bytes or counts for a col-counter                              256
  --help -[h]                             Show this help message and exit                                              true
  --help-config                           Show help message for config properties                                      true
  --quiet                                 Negate verbose                                                               false
  --swc.bkr.host                          Broker Host: "(hostname or ips-csv)|port"                                    []
  --swc.bkr.port                          Broker port                                                                  17000
  --swc.cfg                               Main configuration file                                                      swc.cfg
  --swc.cfg.dyn                           Main dynamic configuration file                                              []
  --swc.cfg.dyn.period                    Dynamic cfg-file check interval in ms, zero without                          600000
  --swc.cfg.path                          Path of configuration files                                                  /opt/swcdb/etc/swcdb/
  --swc.client.Bkr.comm.encoder           The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB         ZSTD  # (4)
  --swc.client.Bkr.connection.keepalive   Broker client connection keepalive for ms since last action                  30000
  --swc.client.Bkr.connection.probes      Broker client connect probes                                                 1
  --swc.client.Bkr.connection.timeout     Broker client connect timeout                                                10000
  --swc.client.Mngr.comm.encoder          The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB         ZSTD  # (4)
  --swc.client.Mngr.connection.keepalive  Manager client connection keepalive for ms since last action                 30000
  --swc.client.Mngr.connection.probes     Manager client connect probes                                                1
  --swc.client.Mngr.connection.timeout    Manager client connect timeout                                               10000
  --swc.client.Mngr.range.master.expiry   Cached Master Range expiry in ms                                             1800000
  --swc.client.Rgr.comm.encoder           The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB         ZSTD  # (4)
  --swc.client.Rgr.connection.keepalive   Ranger client connection keepalive for ms since last action                  30000
  --swc.client.Rgr.connection.probes      Ranger client connect probes                                                 1
  --swc.client.Rgr.connection.timeout     Ranger client connect timeout                                                10000
  --swc.client.Rgr.range.res.expiry       Range Ranger resolution expiry in ms                                         1800000
  --swc.client.recv.ahead                 Client receive number of buffers ahead                                       3
  --swc.client.recv.buffer                Client receive buffer size in bytes                                          8388608
  --swc.client.recv.timeout               Client receive timeout in ms                                                 800000
  --swc.client.request.again.delay        Client request again delay size in ms                                        500
  --swc.client.schema.expiry              Schemas expiry in ms                                                         1800000
  --swc.client.send.ahead                 Client send number of buffers ahead                                          3
  --swc.client.send.buffer                Client send buffer size in bytes                                             8388608
  --swc.client.send.timeout               Client send timeout in ms                                                    800000
  --swc.client.send.timeout.bytes.ratio   Timeout ratio to bytes, bytes/ratio=ms added to send timeout                 1000
  --swc.logging.level -[l]                Logging level: debug|info|notice|warn|error|crit|alert|fatal                 ERROR  # (3)
  --swc.logging.path                      Path of log files                                                            /opt/swcdb/var/log/swcdb/
  --swc.mngr.host                         Manager Host: "[cols range]|(hostname or ips-csv)|port"                      []
  --swc.mngr.port                         Manager default port if not defined in swc.mngr.host                         15000
  --swc.rgr.port                          Ranger port                                                                  16000
  --verbose                               Show more verbose output                                                     false
  --version -[v]                          Show version information and exit                                            true
  --with-broker                           Query applicable requests with Broker                                        false

```


##### GENERATE A LOAD OF SOME SAMPLE DATA

###### run swcdb_load_generator
```bash
./swcdb_load_generator --gen-cells=100000;
```

```text
update-progress(time_ns=1661615696606060316 cells=100000 bytes=32150000 avg=5855ns/cell) Profile(took=585572806ns mngr[locate(time=192219ns count=1 cached=0 errors=0) res(time=500947286ns count=3 cached=0 errors=0)] rgr[locate-master(time=327733ns count=1 cached=0 errors=0) locate-meta(time=492563ns count=2 cached=0 errors=1)])
update-progress(time_ns=1661615696741548153 cells=200000 bytes=64300000 avg=1354ns/cell) Profile(took=721060000ns mngr[locate(time=195840ns count=2 cached=1 errors=0) res(time=500948816ns count=5 cached=2 errors=0)] rgr[locate-master(time=469507ns count=2 cached=0 errors=0) locate-meta(time=583277ns count=3 cached=0 errors=1) data(time=256398647ns count=4 cached=0 errors=0)])
update-progress(time_ns=1661615696951922453 cells=300000 bytes=96450000 avg=2103ns/cell) Profile(took=931434336ns mngr[locate(time=203314ns count=4 cached=3 errors=0) res(time=500951903ns count=9 cached=6 errors=0)] rgr[locate-master(time=728684ns count=4 cached=0 errors=0) locate-meta(time=806143ns count=5 cached=0 errors=1) data(time=713875528ns count=9 cached=0 errors=0)])
update-progress(time_ns=1661615697121940533 cells=400000 bytes=128600000 avg=1700ns/cell) Profile(took=1101452430ns mngr[locate(time=205958ns count=5 cached=4 errors=0) res(time=500953183ns count=11 cached=8 errors=0)] rgr[locate-master(time=949050ns count=5 cached=0 errors=0) locate-meta(time=908180ns count=6 cached=0 errors=1) data(time=993044560ns count=12 cached=0 errors=0)])
update-progress(time_ns=1661615697286786271 cells=500000 bytes=160750000 avg=1648ns/cell) Profile(took=1266298101ns mngr[locate(time=209089ns count=6 cached=5 errors=0) res(time=500954746ns count=13 cached=10 errors=0)] rgr[locate-master(time=1051830ns count=6 cached=0 errors=0) locate-meta(time=987645ns count=7 cached=0 errors=1) data(time=1250664632ns count=15 cached=0 errors=0)])
update-progress(time_ns=1661615697521106661 cells=600000 bytes=192900000 avg=2343ns/cell) Profile(took=1500618706ns mngr[locate(time=215529ns count=8 cached=7 errors=0) res(time=500957831ns count=17 cached=14 errors=0)] rgr[locate-master(time=1396777ns count=8 cached=0 errors=0) locate-meta(time=1234296ns count=9 cached=0 errors=1) data(time=1858158251ns count=21 cached=0 errors=0)])
update-progress(time_ns=1661615697695260924 cells=700000 bytes=225050000 avg=1741ns/cell) Profile(took=1674772729ns mngr[locate(time=221587ns count=9 cached=8 errors=0) res(time=500959479ns count=19 cached=16 errors=0)] rgr[locate-master(time=1583587ns count=9 cached=0 errors=0) locate-meta(time=1316202ns count=10 cached=0 errors=1) data(time=2137999397ns count=24 cached=0 errors=0)])
update-progress(time_ns=1661615697856297783 cells=800000 bytes=257200000 avg=1610ns/cell) Profile(took=1835809569ns mngr[locate(time=225076ns count=10 cached=9 errors=0) res(time=500961271ns count=21 cached=18 errors=0)] rgr[locate-master(time=1703395ns count=10 cached=0 errors=0) locate-meta(time=1400811ns count=11 cached=0 errors=1) data(time=2422985909ns count=27 cached=0 errors=0)])
update-progress(time_ns=1661615698096238737 cells=900000 bytes=289350000 avg=2399ns/cell) Profile(took=2075750691ns mngr[locate(time=241192ns count=12 cached=11 errors=0) res(time=500965147ns count=25 cached=22 errors=0)] rgr[locate-master(time=1979786ns count=12 cached=0 errors=0) locate-meta(time=1578967ns count=13 cached=0 errors=1) data(time=2996305361ns count=33 cached=0 errors=0)])
update-progress(time_ns=1661615698269947193 cells=1000000 bytes=321500000 avg=1736ns/cell) Profile(took=2249459024ns mngr[locate(time=243660ns count=13 cached=12 errors=0) res(time=500966960ns count=27 cached=24 errors=0)] rgr[locate-master(time=2079564ns count=13 cached=0 errors=0) locate-meta(time=1650595ns count=14 cached=0 errors=1) data(time=3298922163ns count=36 cached=0 errors=0)])


Statistics:
 Total Time Took:        2427.23 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       313965 KB
 Average Transfer Rate:  129.351 KB/millisecond
 Average Cells Rate:     411.993 cell/millisecond
 Mngr Locate:            246402ns 14/13/0
 Mngr Resolve:           500968690ns 29/26/0
 Rgr Locate Master:      2174050ns 14/0/0
 Rgr Locate Meta:        1736468ns 15/0/1
 Rgr Data:               3728647177ns 41/0/0
```

###### check on results
```bash
./swcdb;
```

```text
SWC-DB(client)> select where col(load_generator-1)=(cells=([>=0000099998, :<""]<=key<=[<=0000099999, :<""] ONLY_KEYS)) DISPLAY_STATS;
["0000099998"]
["0000099998","0000099998"]
["0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998"]
["0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998","0000099998"]
["0000099999"]
["0000099999","0000099999"]
["0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999"]
["0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999","0000099999"]


Statistics:
 Total Time Took:        1519.21 microseconds
 Total Cells Count:      20
 Total Cells Size:       1450 B
 Average Transfer Rate:  0.954443 B/microsecond
 Average Cells Rate:     0.0131647 cell/microsecond
 Mngr Locate:            118134ns 2/1/1
 Mngr Resolve:           1695ns 2/2/0
 Rgr Locate Master:      453365ns 2/0/1
 Rgr Locate Meta:        320487ns 2/0/1
 Rgr Data:               557537ns 1/0/0

SWC-DB(client)>
```

```text
SWC-DB(client)> select where col(load_generator-1)=(cells=(offset=999999 ONLY_KEYS)) DISPLAY_STATS;
["0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000"]


Statistics:
 Total Time Took:        14.873 milliseconds
 Total Cells Count:      1
 Total Cells Size:       122 B
 Average Transfer Rate:  8.20278 B/millisecond
 Average Cells Rate:     0.0672359 cell/millisecond
 Mngr Locate:            1352603ns/2(1)
 Mngr Resolve:           46960ns/2(0)
 Rgr Locate Master:      1380699ns/2(1)
 Rgr Locate Meta:        1424971ns/2(1)
 Rgr Data:               10572953ns/1(0)

SWC-DB(client)>
```

```text
dump col='load_generator-1' into path='dump/test1/' DISPLAY_STATS;


Statistics:
 Total Time Took:        4005.62 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       321777 KB
 Average Transfer Rate:  80.3314 KB/millisecond
 Average Cells Rate:     249.649 cell/millisecond
 Mngr Locate:            1517560ns/2(1)
 Mngr Resolve:           64729ns/2(0)
 Rgr Locate Master:      1313417ns/2(1)
 Rgr Locate Meta:        1314664ns/2(1)
 Rgr Data:               653997079ns/42(0)

 Files Count:            1
 File:                   dump/test1/1.tsv (367000041 bytes)

SWC-DB(client)>
```

```text
SWC-DB(client)> load from path 'dump/test1/' into col='load_generator-1' DISPLAY_STATS;


Statistics:
 Total Time Took:        2510.37 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       321777 KB
 Average Transfer Rate:  128.179 KB/millisecond
 Average Cells Rate:     398.347 cell/millisecond
 Mngr Locate:            33728436ns/47(0)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      26618866ns/47(0)
 Rgr Locate Meta:        30112961ns/47(0)
 Rgr Data:               3264162871ns/50(0)

SWC-DB(client)>
```


```text
SWC-DB(client)> quit;
```
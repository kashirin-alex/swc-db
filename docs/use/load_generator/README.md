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
update-progress(time_ns=1608138357274470354 cells=100000 bytes=32150000 avg=6162ns/cell) Profile(took=616291400ns mngr[locate(count=1 time=1464208ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=1 time=965690ns errors=0) locate-meta(count=2 time=1139153ns errors=1) data(count=1 time=40440078ns errors=0)])
update-progress(time_ns=1608138357446573123 cells=200000 bytes=64300000 avg=1720ns/cell) Profile(took=788394605ns mngr[locate(count=5 time=4594245ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=5 time=3398183ns errors=0) locate-meta(count=6 time=3589221ns errors=1) data(count=5 time=301061683ns errors=0)])
update-progress(time_ns=1608138357616718404 cells=300000 bytes=96450000 avg=1701ns/cell) Profile(took=958539496ns mngr[locate(count=7 time=5655401ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=7 time=4696201ns errors=0) locate-meta(count=8 time=4858866ns errors=1) data(count=9 time=559957358ns errors=0)])
update-progress(time_ns=1608138357794740315 cells=400000 bytes=128600000 avg=1780ns/cell) Profile(took=1136561436ns mngr[locate(count=9 time=6733974ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=9 time=5925180ns errors=0) locate-meta(count=10 time=5964910ns errors=1) data(count=13 time=795534572ns errors=0)])
update-progress(time_ns=1608138357985374183 cells=500000 bytes=160750000 avg=1906ns/cell) Profile(took=1327195584ns mngr[locate(count=12 time=8481223ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=12 time=7818910ns errors=0) locate-meta(count=13 time=7921328ns errors=1) data(count=17 time=1063245349ns errors=0)])
update-progress(time_ns=1608138358184367680 cells=600000 bytes=192900000 avg=1989ns/cell) Profile(took=1526188854ns mngr[locate(count=14 time=9720490ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=14 time=9014367ns errors=0) locate-meta(count=15 time=9233217ns errors=1) data(count=21 time=1374800120ns errors=0)])
update-progress(time_ns=1608138358375073221 cells=700000 bytes=225050000 avg=1906ns/cell) Profile(took=1716894548ns mngr[locate(count=16 time=10973310ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=16 time=10371553ns errors=0) locate-meta(count=17 time=10509343ns errors=1) data(count=25 time=1606093561ns errors=0)])
update-progress(time_ns=1608138358566580607 cells=800000 bytes=257200000 avg=1914ns/cell) Profile(took=1908402112ns mngr[locate(count=18 time=12347901ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=18 time=11801598ns errors=0) locate-meta(count=19 time=11608841ns errors=1) data(count=29 time=1957149061ns errors=0)])
update-progress(time_ns=1608138358763138597 cells=900000 bytes=289350000 avg=1965ns/cell) Profile(took=2104959996ns mngr[locate(count=20 time=13454399ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=20 time=13130113ns errors=0) locate-meta(count=21 time=12735093ns errors=1) data(count=32 time=2149997715ns errors=0)])
update-progress(time_ns=1608138358986266877 cells=1000000 bytes=321500000 avg=2231ns/cell) Profile(took=2328088036ns mngr[locate(count=23 time=15409865ns errors=0) res(count=3 time=503774681ns errors=0)] rgr[locate-master(count=23 time=14866537ns errors=0) locate-meta(count=24 time=14393134ns errors=1) data(count=37 time=2489937175ns errors=0)])


Statistics:
 Total Time Took:        2468.42 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       313965 KB
 Average Transfer Rate:  127.193 KB/millisecond
 Average Cells Rate:     405.117 cell/millisecond
 Mngr Locate:            16055654ns/24(0)
 Mngr Resolve:           503774681ns/3(0)
 Rgr Locate Master:      15464078ns/24(0)
 Rgr Locate Meta:        14943707ns/25(1)
 Rgr Data:               2784062420ns/41(0)
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
 Total Time Took:        1648.25 microseconds
 Total Cells Count:      20
 Total Cells Size:       1450 B
 Average Transfer Rate:  0.879721 B/microsecond
 Average Cells Rate:     0.0121341 cell/microsecond
 Mngr Locate:            192659ns 2/1/1
 Mngr Resolve:           2566ns 2/2/0
 Rgr Locate Master:      464269ns 2/0/1
 Rgr Locate Meta:        236872ns 2/0/1
 Rgr Data:               671630ns 1/0/0

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
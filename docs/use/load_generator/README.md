---
title: Load Generator
sort: 11
---


# Using SWC-DB load generator

```bash
cd /opt/swcdb/bin;          # if SWCDB_INSTALL_PATH not on PATH
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
# If not, see <https://github.com/kashirin-alex/swc-db/blob/v0.5.11/LICENSE>.

SWC-DB(load_generator) Usage: swcdb_load_generator [options]

Options:
  --daemon                                Start process in background mode                                             true
  --debug                                 Shortcut to --swc.logging.level debug                                        false
  --gen-blk-cells                         Schema blk-cells                                                             0
  --gen-blk-encoding                      Schema blk-encoding NONE|ZSTD|SNAPPY|ZLIB                                    DEFAULT  # (0)
  --gen-blk-size                          Schema blk-size                                                              0
  --gen-cell-a-time                       Write one cell at a time                                                     false
  --gen-cell-encoding                     Cell's Value encoding ZSTD|SNAPPY|ZLIB                                       PLAIN  # (1)
  --gen-cell-versions                     cell key schema-versions                                                     1
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
  --gen-versions                          number of cell versions to generate                                          1
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
update-progress(time_ns=1662200856350434447 cells=100000 bytes=32150000 avg=1230ns/cell) Profile(took=123094744ns mngr[locate(time=384229ns count=2 cached=1 errors=0) res(time=193589ns count=4 cached=2 errors=0)] rgr[locate-master(time=544860ns count=2 cached=0 errors=0) locate-meta(time=270960ns count=2 cached=0 errors=0) data(time=99370301ns count=2 cached=0 errors=0)])
update-progress(time_ns=1662200856472924784 cells=200000 bytes=64300000 avg=1224ns/cell) Profile(took=245585084ns mngr[locate(time=387517ns count=3 cached=2 errors=0) res(time=195235ns count=6 cached=4 errors=0)] rgr[locate-master(time=1028598ns count=3 cached=0 errors=0) locate-meta(time=589187ns count=3 cached=0 errors=0) data(time=353037264ns count=6 cached=0 errors=0)])
update-progress(time_ns=1662200856627764600 cells=300000 bytes=96450000 avg=1548ns/cell) Profile(took=400425002ns mngr[locate(time=390090ns count=4 cached=3 errors=0) res(time=207267ns count=8 cached=6 errors=0)] rgr[locate-master(time=1462719ns count=4 cached=0 errors=0) locate-meta(time=799421ns count=4 cached=0 errors=0) data(time=601726117ns count=9 cached=0 errors=0)])
update-progress(time_ns=1662200856849771384 cells=400000 bytes=128600000 avg=2219ns/cell) Profile(took=622431701ns mngr[locate(time=397929ns count=6 cached=5 errors=0) res(time=210827ns count=12 cached=10 errors=0)] rgr[locate-master(time=2058034ns count=6 cached=0 errors=0) locate-meta(time=1207293ns count=6 cached=0 errors=0) data(time=1080344593ns count=14 cached=0 errors=0)])
update-progress(time_ns=1662200857003870764 cells=500000 bytes=160750000 avg=1540ns/cell) Profile(took=776531032ns mngr[locate(time=400555ns count=7 cached=6 errors=0) res(time=214990ns count=14 cached=12 errors=0)] rgr[locate-master(time=2520326ns count=7 cached=0 errors=0) locate-meta(time=1418932ns count=7 cached=0 errors=0) data(time=1343661789ns count=17 cached=0 errors=0)])
update-progress(time_ns=1662200857170310554 cells=600000 bytes=192900000 avg=1664ns/cell) Profile(took=942970688ns mngr[locate(time=403859ns count=8 cached=7 errors=0) res(time=216516ns count=16 cached=14 errors=0)] rgr[locate-master(time=2713233ns count=8 cached=0 errors=0) locate-meta(time=1605265ns count=8 cached=0 errors=0) data(time=1689149607ns count=21 cached=0 errors=0)])
update-progress(time_ns=1662200857406703841 cells=700000 bytes=225050000 avg=2363ns/cell) Profile(took=1179364449ns mngr[locate(time=413501ns count=10 cached=9 errors=0) res(time=219689ns count=20 cached=18 errors=0)] rgr[locate-master(time=3448076ns count=10 cached=0 errors=0) locate-meta(time=2068540ns count=10 cached=0 errors=0) data(time=2184880167ns count=26 cached=0 errors=0)])
update-progress(time_ns=1662200857572139158 cells=800000 bytes=257200000 avg=1654ns/cell) Profile(took=1344799270ns mngr[locate(time=416069ns count=11 cached=10 errors=0) res(time=230608ns count=22 cached=20 errors=0)] rgr[locate-master(time=3712666ns count=11 cached=0 errors=0) locate-meta(time=2330731ns count=11 cached=0 errors=0) data(time=2481739508ns count=29 cached=0 errors=0)])
update-progress(time_ns=1662200857808962288 cells=900000 bytes=289350000 avg=2368ns/cell) Profile(took=1581622510ns mngr[locate(time=422323ns count=13 cached=12 errors=0) res(time=234910ns count=26 cached=24 errors=0)] rgr[locate-master(time=4403586ns count=13 cached=0 errors=0) locate-meta(time=2851130ns count=13 cached=0 errors=0) data(time=3051886523ns count=35 cached=0 errors=0)])
update-progress(time_ns=1662200857979236031 cells=1000000 bytes=321500000 avg=1702ns/cell) Profile(took=1751896577ns mngr[locate(time=424840ns count=14 cached=13 errors=0) res(time=236885ns count=28 cached=26 errors=0)] rgr[locate-master(time=4643211ns count=14 cached=0 errors=0) locate-meta(time=3204782ns count=14 cached=0 errors=0) data(time=3336224317ns count=38 cached=0 errors=0)])


Statistics:
 Total Time Took:        1909.74 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       313965 KB
 Average Transfer Rate:  164.402 KB/millisecond
 Average Cells Rate:     523.632 cell/millisecond
 Mngr Locate:            427348ns 15/14/0
 Mngr Resolve:           238629ns 30/28/0
 Rgr Locate Master:      4870678ns 15/0/0
 Rgr Locate Meta:        3392862ns 15/0/0
 Rgr Data:               3726139865ns 43/0/0
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
 Total Time Took:        1328.13 microseconds
 Total Cells Count:      20
 Total Cells Size:       1450 B
 Average Transfer Rate:  1.09176 B/microsecond
 Average Cells Rate:     0.0150587 cell/microsecond
 Mngr Locate:            107979ns 2/1/1
 Mngr Resolve:           1565ns 2/2/0
 Rgr Locate Master:      324185ns 2/0/1
 Rgr Locate Meta:        398788ns 2/0/1
 Rgr Data:               433068ns 1/0/0

SWC-DB(client)>
```

```text
SWC-DB(client)> select where col(load_generator-1)=(cells=(offset=999999 ONLY_KEYS)) DISPLAY_STATS;
["0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000","0000100000"]


Statistics:
 Total Time Took:        43.3204 milliseconds
 Total Cells Count:      1
 Total Cells Size:       122 B
 Average Transfer Rate:  2.81622 B/millisecond
 Average Cells Rate:     0.0230838 cell/millisecond
 Mngr Locate:            136503ns 2/1/1
 Mngr Resolve:           1292ns 2/2/0
 Rgr Locate Master:      576250ns 2/0/1
 Rgr Locate Meta:        657652ns 2/0/1
 Rgr Data:               41896503ns 1/0/0

SWC-DB(client)>
```

```text
dump col='load_generator-1' into path='dump/test1/' DISPLAY_STATS;


Statistics:
 Total Time Took:        14.3019 seconds
 Total Cells Count:      1000000
 Total Cells Size:       321777 KB
 Average Transfer Rate:  22499 KB/second
 Average Cells Rate:     69920.9 cell/second
 Mngr Locate:            143964ns 2/1/1
 Mngr Resolve:           1582ns 2/2/0
 Rgr Locate Master:      369965ns 2/0/1
 Rgr Locate Meta:        258151ns 2/0/1
 Rgr Data:               1188184872ns 42/0/0

 Files Count:            1
 File:                   dump/test1/1.tsv (367000041 bytes)

SWC-DB(client)>
```

```text
SWC-DB(client)> load from path='dump/test1/' into col='load_generator-1' DISPLAY_STATS;


Statistics:
 Total Time Took:        2500.88 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       321777 KB
 Average Transfer Rate:  128.665 KB/millisecond
 Average Cells Rate:     399.858 cell/millisecond
 Mngr Locate:            54666ns 16/16/0
 Mngr Resolve:           33632ns 32/32/0
 Rgr Locate Master:      2303200ns 16/0/0
 Rgr Locate Meta:        1862946ns 16/0/0
 Rgr Data:               5138617081ns 48/0/0

SWC-DB(client)>
```


```text
SWC-DB(client)> quit;
```

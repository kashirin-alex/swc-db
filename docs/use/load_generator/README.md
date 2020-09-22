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
# If not, see <https://github.com/kashirin-alex/swc-db/blob/master/LICENSE>.

SWC-DB(load_generator) Usage: swcdb_load_generator [options]

Options:
  --daemon                                Start process in background mode                                             true
  --debug                                 Shortcut to --swc.logging.level debug                                        false
  --gen-blk-cells                         Schema blk-cells                                                             0
  --gen-blk-encoding                      Schema blk-encoding NONE/ZSTD/SNAPPY/ZLIB                                    DEFAULT  # (0)
  --gen-blk-size                          Schema blk-size                                                              0
  --gen-cell-a-time                       Write one cell at a time                                                     false
  --gen-cell-versions                     cell key versions                                                            1
  --gen-cells                             number of cells, total=cells*versions*(key-tree? key-fractions : 1)          1000
  --gen-col-name                          Gen. load column name                                                        load_generator
  --gen-col-seq                           Schema col-seq FC_+/LEXIC/VOLUME                                             LEXIC  # (1)
  --gen-col-type                          Schema col-type PLAIN/COUNTER_I64/COUNTER_I32/COUNTER_I16/COUNTER_I8         PLAIN  # (1)
  --gen-compaction-percent                Compaction threshold in % applied over size of either by cellstore or block  0
  --gen-cs-count                          Schema cs-count                                                              0
  --gen-cs-replication                    Schema cs-replication                                                        0
  --gen-cs-size                           Schema cs-size                                                               0
  --gen-delete                            Delete generated data                                                        false
  --gen-delete-column                     Delete Column after                                                          false
  --gen-fraction-size                     fraction size in bytes at least                                              10
  --gen-insert                            Generate new data                                                            true
  --gen-key-fractions                     Number of Fractions per cell key                                             10
  --gen-key-tree                          Key Fractions in a tree form [1], [1, 2]                                     true
  --gen-log-rollout                       CommitLog rollout block ratio                                                0
  --gen-progress                          display progress every N cells or 0 for quiet                                100000
  --gen-reverse                           Generate in reverse, always writes to 1st range                              false
  --gen-select                            Select generated data                                                        false
  --gen-select-empty                      Expect empty select results                                                  false
  --gen-value-size                        cell value in bytes or counts for a col-counter                              256
  --help -[h]                             Show this help message and exit                                              true
  --help-config                           Show help message for config properties                                      true
  --quiet                                 Negate verbose                                                               false
  --swc.cfg                               Main configuration file                                                      swc.cfg
  --swc.cfg.dyn                           Main dynamic configuration file                                              []
  --swc.cfg.dyn.period                    Dynamic cfg-file check interval in ms, zero without                          600000
  --swc.cfg.path                          Path of configuration files                                                  /opt/swcdb/etc/swcdb/
  --swc.client.Mngr.connection.keepalive  Manager client connection keepalive for ms since last action                 30000
  --swc.client.Mngr.connection.probes     Manager client connect probes                                                1
  --swc.client.Mngr.connection.timeout    Manager client connect timeout                                               10000
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

```


##### GENERATE A LOAD OF SOME SAMPLE DATA

###### run swcdb_load_generator
```bash
./swcdb_load_generator --gen-cells=100000;
```

```text
 progress(cells=100000 bytes=32150000 cell/ns=1784) Profile(took=0ns mngr[locate(count=5 time=922339ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=5 time=854258ns errors=0) locate-meta(count=5 time=983112ns errors=0) data(count=4 time=111161626ns errors=0)])
 progress(cells=200000 bytes=64300000 cell/ns=1732) Profile(took=0ns mngr[locate(count=9 time=1431793ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=9 time=1272753ns errors=0) locate-meta(count=9 time=1375490ns errors=0) data(count=8 time=267873288ns errors=0)])
 progress(cells=300000 bytes=96450000 cell/ns=1736) Profile(took=0ns mngr[locate(count=12 time=1827944ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=12 time=1603651ns errors=0) locate-meta(count=12 time=1673478ns errors=0) data(count=11 time=411488019ns errors=0)])
 progress(cells=400000 bytes=128600000 cell/ns=1949) Profile(took=0ns mngr[locate(count=14 time=2144028ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=14 time=1883633ns errors=0) locate-meta(count=14 time=1929977ns errors=0) data(count=15 time=656728895ns errors=0)])
 progress(cells=500000 bytes=160750000 cell/ns=2023) Profile(took=0ns mngr[locate(count=18 time=2699469ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=18 time=2370183ns errors=0) locate-meta(count=18 time=2375885ns errors=0) data(count=19 time=902771091ns errors=0)])
 progress(cells=600000 bytes=192900000 cell/ns=1958) Profile(took=0ns mngr[locate(count=20 time=2974040ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=20 time=2634322ns errors=0) locate-meta(count=20 time=2586125ns errors=0) data(count=23 time=1158093212ns errors=0)])
 progress(cells=700000 bytes=225050000 cell/ns=1983) Profile(took=0ns mngr[locate(count=22 time=3271112ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=22 time=2866603ns errors=0) locate-meta(count=22 time=2784441ns errors=0) data(count=26 time=1410341485ns errors=0)])
 progress(cells=800000 bytes=257200000 cell/ns=2019) Profile(took=0ns mngr[locate(count=24 time=3538260ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=24 time=3111473ns errors=0) locate-meta(count=24 time=3018799ns errors=0) data(count=30 time=1710843676ns errors=0)])
 progress(cells=900000 bytes=289350000 cell/ns=1906) Profile(took=0ns mngr[locate(count=26 time=3781197ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=26 time=3336873ns errors=0) locate-meta(count=26 time=3233807ns errors=0) data(count=34 time=2000196536ns errors=0)])
 progress(cells=1000000 bytes=321500000 cell/ns=3169) Profile(took=1956855249ns mngr[locate(count=28 time=4193694ns errors=0) res(count=2 time=211156ns errors=0)] rgr[locate-master(count=28 time=3788386ns errors=0) locate-meta(count=28 time=3560372ns errors=0) data(count=40 time=2546827778ns errors=0)])


Statistics:
 Total Time Took:        2096.55 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       313965 KB
 Average Transfer Rate:  149.753 KB/millisecond
 Average Cells Rate:     476.975 cell/millisecond
 Mngr Locate:            4347083ns/29(0)
 Mngr Resolve:           211156ns/2(0)
 Rgr Locate Master:      3916771ns/29(0)
 Rgr Locate Meta:        3661841ns/29(0)
 Rgr Data:               2643616107ns/42(0)
```

###### check on results
```bash
./swcdb;
```

```text
SWC-DB(client)> select where col(load_generator)=(cells=([0000099999, >=""]<=key<=[0000099999, >=""] ONLY_KEYS)) DISPLAY_STATS;
[0000099999]
[0000099999, 0000000001]
[0000099999, 0000000001, 0000000002]
[0000099999, 0000000001, 0000000002, 0000000003]
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004]
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005]
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005, 0000000006]
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005, 0000000006, 0000000007]
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005, 0000000006, 0000000007, 0000000008]
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005, 0000000006, 0000000007, 0000000008, 0000000009]


Statistics:
 Total Time Took:        4870.74 microseconds
 Total Cells Count:      10
 Total Cells Size:       725 B
 Average Transfer Rate:  0.148848 B/microsecond
 Average Cells Rate:     0.00205308 cell/microsecond
 Mngr Locate:            353793ns/2(1)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      352739ns/2(1)
 Rgr Locate Meta:        330608ns/2(1)
 Rgr Data:               3737728ns/1(0)
```

```text
SWC-DB(client)> select where col(load_generator)=(cells=(offset=999999 ONLY_KEYS)) DISPLAY_STATS;
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005, 0000000006, 0000000007, 0000000008, 0000000009]


Statistics:
 Total Time Took:        18.475 milliseconds
 Total Cells Count:      1
 Total Cells Size:       122 B
 Average Transfer Rate:  6.60352 B/millisecond
 Average Cells Rate:     0.0541272 cell/millisecond
 Mngr Locate:            513113ns/2(1)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      422130ns/2(1)
 Rgr Locate Meta:        433575ns/2(1)
 Rgr Data:               17029467ns/1(0)
```

```text
SWC-DB(client)> dump col='load_generator' into 'dump/test1/' DISPLAY_STATS;


Statistics:
 Total Time Took:        4121.59 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       321777 KB
 Average Transfer Rate:  78.0712 KB/millisecond
 Average Cells Rate:     242.625 cell/millisecond
 Mngr Locate:            443459ns/2(1)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      438194ns/2(1)
 Rgr Locate Meta:        386816ns/2(1)
 Rgr Data:               1086274646ns/42(0)

 Files Count:            1
 File:                   dump/test1/1.tsv (367000041 bytes)
```

```text
SWC-DB(client)> load from 'dump/test1/' into col='load_generator' DISPLAY_STATS;


Statistics:
 Total Time Took:        2626.23 milliseconds
 Total Cells Count:      1000000
 Total Cells Size:       321777 KB
 Average Transfer Rate:  122.525 KB/millisecond
 Average Cells Rate:     380.774 cell/millisecond
 Mngr Locate:            8227547ns/31(0)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      7937411ns/31(0)
 Rgr Locate Meta:        5094435ns/31(0)
 Rgr Data:               3041157529ns/42(0)

SWC-DB(client)>;
```


```text
SWC-DB(client)> quit;
```
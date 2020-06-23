# SWC-DB (Super Wide Column Database)


#### Work in-progress

### ABSTRACT
https://alex.kashirin.family/swc-DB.pdf


### BUILD & INSTALL

* Prerequisites 
  * [GCC c++2a+](https://gcc.gnu.org/)
  * [ASIO 1.16+](https://github.com/chriskohlhoff/asio)
  * [libtcmalloc](https://github.com/gperftools/gperftools) - optional
  * [libre2](https://github.com/google/re2)
  * [libsnappy](https://github.com/google/snappy)
  * [libzlib](https://www.zlib.net/)
  * [libzstd](https://github.com/facebook/zstd)
  * [libopenssl TLS-1.3](https://www.openssl.org/)
  * [libeditline](https://github.com/troglobit/editline) 
  * Hadoop-JVM FsBroker requires - optional:
    * [Apache-Hadoop + libhdfs](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfs)
    * [Java(openjdk 12.0.1) - libjvm, libjava, libverify](https://jdk.java.net/java-se-ri/12)
  * Hadoop FsBroker requires - optional:
    * [Apache-Hadoop + libhdfspp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)
    * [libprotobuf](https://github.com/protocolbuffers/protobuf)
  * Thrift Broker requires - optional:
    * [Apache Thrift 0.13.0+](https://github.com/apache/thrift)
    * [libevent 2.1.11+](http://github.com/libevent/libevent)
  * Thrift c_glib client & PAM module, require - optional:
    * [libglib-2.0](https://developer.gnome.org/glib/2.64/)
    * [libgobject-2.0](https://developer.gnome.org/gobject/2.64/)
    * [libffi](http://github.com/libffi/libffi/)
    * [libpcre 1](https://pcre.org/)
  * [fabric-pylib](https://github.com/fabric/fabric) - optional (sbin/swcdb_cluster)
  * [doxygen](https://github.com/doxygen/doxygen) - optional
  * *-DO_LEVEL > 4 require static libraries*

```bash
git clone https://github.com/kashirin-alex/swc-db.git
mkdir swcdb; cd swcdb;
```

```cmake
cmake ../swc-db \
 -DO_LEVEL=6 -DSWC_IMPL_SOURCE=ON \
 -DSWC_BUILTIN_FS=local,broker -DSWC_LANGUAGES=ALL \
 -DCMAKE_SKIP_RPATH=OFF -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
 -DSWC_DOCUMENTATION=OFF \
 -DCMAKE_BUILD_TYPE=Release
```
```bash
make -j8 install
```

#### STARTING-UP
Edit the necessary configuration in /opt/swcdb/etc/swcdb/*.cfg

###### STANDALONE
```bash
cd /opt/swcdb;
mkdir -p var/log/swcdb; # re-config "swc.logging.path" for other path
```
> start as a local cluster
```bash
cd sbin;
./swcdb_cluster start;
```
or 
> start independently 
```bash
cd bin;
# START SWCDB-FS-BROKER 
./swcdbFsBroker --daemon;
# START SWCDB-MANAGER
./swcdbManager --debug --host=localhost --daemon;
# START SWCDB-RANGER
./swcdbRanger --daemon;
# START SWCDB-THRIFT-BROKER
./swcdbThriftBroker --host=localhost --daemon;
```

###### DISTRIBUTED CLUSTER
configure /opt/swcdb/etc/swcdb/swc.cluster.cfg
```bash
cd /opt/swcdb/sbin/;
./swcdb_cluster --help; # for available tasks:
./swcdb_cluster deploy 
./swcdb_cluster start
./swcdb_cluster stop
```

or on each server:
```bash
cd /opt/swcdb;
mkdir -p var/log/swcdb; # re-config "swc.logging.path" for other path
cd bin;
# START SWCDB-FS-BROKER 
./swcdbFsBroker --daemon;
# START SWCDB-MANAGER
./swcdbManager --debug --daemon; # (on the corresponing hosts of "swc.mngr.host" in cfg)
# START SWCDB-RANGER
./swcdbRanger --daemon;
# START SWCDB-THRIFT-BROKER
./swcdbThriftBroker --host=localhost --daemon;
```


#### RUNNING & USING

##### ENTER SWC-DB(client) CLI:
```bash
./swcdb;
```


##### READ "Usage Help:;
```SQL
SWC-DB(client)> help;
Usage Help:  'command' [options];
  quit             Quit or Exit the Console
  help             Commands help information
  add column       add column|schema (schema definitions [name=value ]);
  modify column    modify column|schema (schema definitions [name=value ]);
  delete column    delete column|schema (schema definitions [name=value ]);
  list columns     list|get column|s [NAME|ID,..];
  select           select where [Columns[Cells[Interval Flags]]] Flags DisplayFlags;
                   -> select where COL(NAME|ID,) = ( cells=(Interval Flags) ) AND
                        COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(
                          [F-begin] <= range <= [F-end]                   AND
                          [COMP 'F-start'] <=  key  <= [COMP 'F-finish']  AND
                          'TS-begin' <= timestamp <= 'TS-finish'          AND
                          offset_key = [F] offset_rev='TS'                AND
                          value COMP 'DATA'
                          LIMIT=NUM   OFFSET=NUM  ONLY_KEYS   ONLY_DELETES     )
                        ) DISPLAY_* TIMESTAMP / DATETIME / SPECS / STATS / BINARY;
  update           update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE), CELL(..)      ;
                   -> UPDATE
                        cell(DELETE,                  CID, ['K','E','Y']             );
                        cell(DELETE_VERSION,          CID, ['K','E','Y'], TS         );
                        cell(DELETE_FRACTION,         CID, ['K','E','Y']             );
                        cell(DELETE_FRACTION_VERSION, CID, ['K','E'],     TS         );
                        cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, ''),
                        cell(INSERT,                  CID, ['K','E','Y'], DESC       ),
                        cell(INSERT,                 NAME, ['K','E','Y'], '', 'DATA' ),
                        cell(INSERT_FRACTION,        NAME, ['K','E'],     '', 'DATA' );
                    Flags: INSERT|1 DELETE|2 DELETE_VERSION|3
                           INSERT_FRACTION|4 DELETE_FRACTION|5 DELETE_FRACTION_VERSION|6
  dump             dump col='ID|NAME' into 'file.ext' where [cells=(Interval Flags) AND] OutputFlags DisplayFlags;
                   -> dump col='ColName' into 'ColName.tsv' OUTPUT_NO_* TS / VALUE;
  load             load from 'file.ext' into col='ID|NAME' DisplayFlags;
SWC-DB(client)> quit;

```


##### Making your first column
```SQL
SWC-DB(client)> add column(name=FirstColumn);
SWC-DB(client)> get column FirstColumn;
cid=N name="FirstColumn" type=PLAIN revision=N compact=0 cell_versions=1 cell_ttl=0 
blk_replication=0 blk_encoding=DEFAULT blk_size=0 blk_cells=0 cs_size=0 cs_max=0
SWC-DB(client)> quit;
```


##### INSERT Your First SWC-DB cell
```SQL
SWC-DB(client)> update cell(INSERT, FirstColumn, [My, First, Super, Wide, Column, Key, Fractions], "", "The Cell Data Value") DISPLAY_STATS;


Statistics:
 Total Time Took:        834.152 microseconds
 Total Cells Count:      1
 Total Cells Size:       64 B
 Average Transfer Rate:  0.0767246 B/microsecond
 Average Cells Rate:     0.00119882 cell/microsecond
 Mngr Locate:            203563ns/1(0)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      247751ns/1(0)
 Rgr Locate Meta:        190690ns/1(0)
 Rgr Data:               157399ns/1(0)

SWC-DB(client)> quit;
```


##### SELECT cells
```SQL
SWC-DB(client)> select where col(FirstColumn)=( cells=() ) DISPLAY_STATS DISPLAY_TIMESTAMP DISPLAY_DATETIME;
2020/01/23 19:58:09.820320533  1579809489820320533  [My, First, Super, Wide, Column, Key, Fractions]  The Cell Data Value


Statistics:
 Total Time Took:        1012.45 microseconds
 Total Cells Count:      1
 Total Cells Size:       72 B
 Average Transfer Rate:  0.0711147 B/microsecond
 Average Cells Rate:     0.000987704 cell/microsecond
 Mngr Locate:            318855ns/2(1)
 Mngr Resolve:           0ns/0(0)
 Rgr Locate Master:      278175ns/2(1)
 Rgr Locate Meta:        238001ns/2(1)
 Rgr Data:               124960ns/1(0)

SWC-DB(client)> quit;
```


##### GENERATE A LOAD OF SOME SAMPLE DATA

###### enter help
```bash
./swcdb_load_generator --help
SWC-DB(load_generator) Usage: swcdb_load_generator [options]

Options:
 --debug                   Shortcut to --swc.logging.level debug                                        false
 --gen-blk-cells           Schema blk-cells                                                             0
 --gen-blk-encoding        Schema blk-encoding NONE/SNAPPY/ZLIB                                         DEFAULT  # (0)
 --gen-blk-size            Schema blk-size                                                              0
 --gen-cell-versions       cell key versions                                                            1
 --gen-cells               number of cells, total=cells*versions*(key-tree? key-fractions : 1)          1000
 --gen-col-name            Gen. load column name                                                        load_generator
 --gen-col-type            Schema col-type PLAIN/COUNTER_I64/COUNTER_I32/COUNTER_I16/COUNTER_I8         PLAIN  # (1)
 --gen-compaction-percent  Compaction threshold in % applied over size of either by cellstore or block  0
 --gen-cs-count            Schema cs-count                                                              0
 --gen-cs-replication      Schema cs-replication                                                        0
 --gen-cs-size             Schema cs-size                                                               0
 --gen-fraction-size       fraction size in bytes at least                                              10
 --gen-key-fractions       Number of Fractions per cell key                                             10
 --gen-key-tree            Key Fractions in a tree form [1], [1, 2]                                     true
 --gen-progress            display progress every N cells or 0 for quite                                100000
 --gen-reverse             Generate in reverse, always writes to 1st range                              false
 --gen-value-size          cell value in bytes or counts for a col-counter                              256
```

###### run swcdb_load_generator
```bash
./swcdb_load_generator --gen-cells=100000;
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
```SQL
./swcdb;
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

SWC-DB(client)> quit;
```


#### SHUTTING-DOWN
> by sending a SIGINT
```bash
ps aux | grep swcdbRanger
kill pid; # -9 for instant/ungracefull shutdown
the same for swcdbManager, swcdbFsBroker and swcdbThriftBroker
```
> by stop command with swcdb_cluster on a local/distributed cluster
```bash
/opt/swcdb/sbin/swcdb_cluster stop
```



### THAT IS ALL THE GUIDE FOR NOW - [WIKI TO COME](https://github.com/kashirin-alex/swc-db/wiki)

##### SUPPORT & DISUCSSIONS
Google Group is available at [groups.google.com/forum/#!forum/swc-db](https://groups.google.com/forum/#!forum/swc-db) for open discussions and help on SWC-DB

##### ISSUES
open an issue at [github.com/kashirin-alex/swc-db/issues](https://github.com/kashirin-alex/swc-db/issues) in case there is an issue(bug/feature) that can be fully described.



### LICENSE

    SWC-DB Copyright (C) since 2019 [author: Kashirin Alex kashirin.alex@gmail.com]
    SWC-DB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, GPLv3 version.

    SWC-DB© is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://github.com/kashirin-alex/swc-db/blob/master/LICENSE>.


###### Non-Commercial License

    No other License available

###### Commercial License

    For a Commercial Purpose License, in-case GPLv3 License limit your commercial activites.
    You are welcome send an email to kashirin.alex@gmail.com with your Commercial requirements.


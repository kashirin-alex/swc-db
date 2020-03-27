# SWC-DB (Super Wide Column Database)


#### Work in-progress

### ABSTRACT
https://alex.kashirin.family/swc-DB.pdf


### BUILD & INSTALL

* Prerequisites 
  * [GCC c++17+](https://gcc.gnu.org/)
  * [ASIO 1.14+](https://github.com/chriskohlhoff/asio)
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

#### STARING-UP
Edit the necessary configuration in /opt/swcdb/etc/swcdb/*.cfg

###### STANDALONE
```bash
cd /opt/swcdb;
mkdir -p var/log/swcdb; # re-config "swc.logging.path" for other path
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
On each server:
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
 Total Time Took:        877.877 us
 Total Cells Count:      1
 Total Cells Size:       64 B
 Average Transfer Rate:  0.0729032 B/us
 Average Cells Rate:     0.00113911 cell/us
SWC-DB(client)> quit;
```


##### SELECT cells
```SQL
SWC-DB(client)> select where col(FirstColumn)=( cells=() ) DISPLAY_STATS DISPLAY_TIMESTAMP DISPLAY_DATETIME;
2020/01/23 19:58:09.820320533  1579809489820320533  [My, First, Super, Wide, Column, Key, Fractions]  The Cell Data Value


Statistics:
 Total Time Took:        1739.04 us
 Total Cells Count:      1
 Total Cells Size:       72 B
 Average Transfer Rate:  0.0414021 B/us
 Average Cells Rate:     0.000575029 cell/us
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
 progress(cells=100000 bytes=32150000 cell/ns=2058)
 progress(cells=200000 bytes=64300000 cell/ns=1768)
 progress(cells=300000 bytes=96450000 cell/ns=1688)
 progress(cells=400000 bytes=128600000 cell/ns=3659)
 progress(cells=500000 bytes=160750000 cell/ns=1695)
 progress(cells=600000 bytes=192900000 cell/ns=1958)
 progress(cells=700000 bytes=225050000 cell/ns=3747)
 progress(cells=800000 bytes=257200000 cell/ns=1921)
 progress(cells=900000 bytes=289350000 cell/ns=2009)
 progress(cells=1000000 bytes=321500000 cell/ns=3789)


Statistics:
 Total Time Took:        2511.05 ms
 Total Cells Count:      1000000
 Total Cells Size:       321500 KB
 Average Transfer Rate:  128.034 KB/ms
 Average Cells Rate:     398.239 cell/ms
```
###### check on results
```SQL
./swcdb;
SWC-DB(client)> select where col(load_generator)=(cells=([0000099999, >""]<=key<=[0000099999, >""] limit=9 ONLY_KEYS)) DISPLAY_STATS;
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
 Total Time Took:        7919.27 us
 Total Cells Count:      9
 Total Cells Size:       702 B
 Average Transfer Rate:  0.0886446 B/us
 Average Cells Rate:     0.00113647 cell/us
SWC-DB(client)> select where col(GenData2)=(cells=(offset=999999 ONLY_KEYS)) DISPLAY_STATS;
[0000099999, 0000000001, 0000000002, 0000000003, 0000000004, 0000000005, 0000000006, 0000000007, 0000000008, 0000000009]


Statistics:
 Total Time Took:        43.923 ms
 Total Cells Count:      1
 Total Cells Size:       122 B
 Average Transfer Rate:  2.77759 B/ms
 Average Cells Rate:     0.0227671 cell/ms
SWC-DB(client)> quit;
```


#### SHUTTING-DOWN
```bash
ps aux | grep swcdbRanger
kill pid; # -9 for instant/ungracefull shutdown
the same for swcdbManager, swcdbFsBroker and swcdbThriftBroker
```


### THAT IS ALL THE GUIDE FOR NOW - [WIKI TO COME](https://github.com/kashirin-alex/swc-db/wiki)

##### SUPPORT & DISUCSSIONS
Google Group is available at [groups.google.com/forum/#!forum/swc-db](https://groups.google.com/forum/#!forum/swc-db) for open discussions and help on SWC-DB

##### ISSUES
open an issue at [github.com/kashirin-alex/swc-db/issues](https://github.com/kashirin-alex/swc-db/issues) in case there is an issue(bug/feature) that can be fully described.
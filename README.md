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
  * [libopenssl tls1.2+](https://www.openssl.org/)
  * [libeditline](https://github.com/troglobit/editline) 
  * Hadoop-JVM FsBroker requires - optional:
    * [Apache-Hadoop + libhdfs](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfs)
    * [Java(openjdk 12.0.1) - libjvm, libjava, libverify](https://jdk.java.net/java-se-ri/12)
  * Hadoop FsBroker requires - optional:
    * [Apache-Hadoop + libhdfspp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)
    * [libprotobuf](https://github.com/protocolbuffers/protobuf)
  * (static version libraries required for -DO_LEVEL > 4)


```bash
git clone https://github.com/kashirin-alex/swc-db.git
mkdir swcdb; cd swcdb;
```

```cmake
cmake ../swc-db \
  -DO_LEVEL=6 -DSWC_IMPL_SOURCE=ON -DSWC_BUILTIN_FS=local,broker \
  -DCMAKE_SKIP_RPATH=OFF -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
  -DCMAKE_BUILD_TYPE=Release
```
```bash
make -j8 install
```

#### RUNNING - STANDALONE
Edit the necessary configuration in /opt/swcdb/etc/swcdb/*.cfg

```bash
cd /opt/swcdb;
mkdir -p var/log/swcdb; # re-conf "swc.logging.path" for other path
cd bin;
# START SWCDB-FS-BROKER 
./swcdbFsBroker --daemon;
# START SWCDB-MANAGER
./swcdbManager --debug --host=localhost --daemon;
# START SWCDB-RANGER
./swcdbRanger --swc.rgr.ram.percent=3 --daemon;
```


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
SWC-DB(client)>

```


##### Making your first column
```SQL
SWC-DB(client)> add column(name=FirstColumn);
SWC-DB(client)> get column FirstColumn;
cid=N name="FirstColumn" type=PLAIN revision=N compact=0 cell_versions=1 cell_ttl=0 
blk_replication=0 blk_encoding=DEFAULT blk_size=0 blk_cells=0 cs_size=0 cs_max=0
SWC-DB(client)> 
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
SWC-DB(client)>
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
SWC-DB(client)>
```

#### SHUTTING-DOWN
```bash
ps aux | grep swcdbRanger
kill pid; # -9 for instant/ungracefull shutdown
the same for swcdbManager and swcdbFsBroker 
```


### THAT IS ALL THE GUIDE FOR NOW - [WIKI TO COME](https://github.com/kashirin-alex/swc-db/wiki)
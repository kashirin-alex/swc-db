---
title: CLI SWC-DB(client)
sort: 1
---


# Using the SWC-DB(client) CLI - The SWC-DB DB-Client Shell
The DB-Client is a client implementing the [SQL]({{ site.baseurl }}/use/sql/) commands and syntax whereas it has additional Flags and commands.

* [A Backup & Restore procedure](backup_restore/)


##### ENTER SWC-DB(client) CLI:
```bash
cd ${SWCDB_INSTALL_PATH}/bin;

./swcdb;
```


##### Enter Help:

```bash
SWC-DB(client)> help;
```

```bash
Usage Help:  'command' [options];
  quit              Quit or Exit the Console
  help              Commands help information
  switch to         Switch to other CLI, options: rgr|mngr|fs|stats|client
  add column        add column|schema (schema definitions [name=value ]);
  modify column     modify column|schema (schema definitions [name=value ]);
  delete column     delete column|schema (schema definitions [name=value ]);
  list columns      list|get column|s [OUTPUT_FLAGS]
                         [(NAME|ID),.., Comp'expr',.., tags Comp[Comp'expr',..]];
                    * OUTPUT_FLAGS: OUTPUT_ONLY_CID
  compact column    compact column|s
                         [(NAME|ID),.., Comp'expr',.., tags Comp[Comp'expr',..]];
  select            select where [Columns[Cells[Interval Flags]]] Flags DisplayFlags;
                    -> select where COL(NAME|ID,.,Comp'expr',.,tags Comp[Comp'expr',..])
                                     = (cells=(Interval Flags)) AND
                         COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(
                           [F-begin] <= range <= [F-end]                   AND
                           [[COMP 'F-start'] <=  key  <= [COMP 'F-finish'] AND]
                           'TS-begin' <= timestamp <= 'TS-finish'          AND
                           offset_key = [F] offset_rev='TS'                AND
                           value COMP 'DATA'
                           LIMIT=NUM OFFSET=NUM MAX_VERSIONS=NUM ONLY_KEYS ONLY_DELETES)
                         ) DISPLAY_* TIMESTAMP, DATETIME, SPECS, STATS, BINARY, COLUMN;
                    * DATA-value: PLAN, COUNTER, SERIAL([ID:TYPE:COMP "VALUE", ..])
  update            update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE, ENC), CELL(..) ;
                    -> UPDATE cell(DELETE_LE,  CID, ['K','E','Y']              ),
                              cell(DELETE_EQ,  CID, ['K','E','Y'], TS          ),
                              cell(INSERT,     CID, ['K','E','Y'], ASC, TS, '' ),
                              cell(INSERT,     CID, ['K','E','Y'], DESC        ),
                              cell(INSERT,     NAME, ['K','E','Y'], '', 'DATA' );
                    * FLAG: INSERT|1 DELETE_LE|2 DELETE_EQ|3
                    * Encoder(ENC): at INSERT with DATA, options: ZLIB|2 SNAPPY|3 ZSTD|4
                    * DATA: PLAIN( val ) COUNTER( -/+/=val ) SERIAL( [ID:TYPE:val, ..] )
  dump              dump col='ID|NAME' into [FS] path='folder/path/' [FORMAT]
                       where [cells=(Interval Flags) AND .. ] OutputFlags DisplayFlags;
                    -> dump col='ColName' into fs=hadoop_jvm path='FolderName'
                         split=1GB ext=zst level=6 OUTPUT_NO_* TS/VALUE|ENCODE;
                    * FS optional: [fs=Type] Write to the specified Type
                    * FORMAT optional: split=1GB ext=zst level=INT(ext-dependent)
  load              load from [FS] path='folder/path/' into col='ID|NAME' DisplayFlags;
                    * FS optional: [fs=Type] Read from the specified Type

SWC-DB(client)>
```


##### Making your first column
```bash
SWC-DB(client)> add column(name=FirstColumn);
SWC-DB(client)> get column FirstColumn;
Schema(cid=10 name="FirstColumn" tags=[] seq=LEXIC type=PLAIN revision=1630745999588173486 compact=0 cell_versions=1 cell_ttl=0 blk_encoding=DEFAULT blk_size=0 blk_cells=0 cs_replication=0 cs_size=0 cs_max=0 log_rollout=0 log_compact=0 log_preload=0)
SWC-DB(client)>
```


##### INSERT Your First SWC-DB cell
```bash
SWC-DB(client)> update cell(INSERT, FirstColumn, [My, First, Super, Wide, Column, Key, Fractions], "", "The Cell Data Value") DISPLAY_STATS;


Statistics:
Statistics:
 Total Time Took:        456.938 microseconds
 Total Cells Count:      1
 Total Cells Size:       64 B
 Average Transfer Rate:  0.140063 B/microsecond
 Average Cells Rate:     0.00218848 cell/microsecond
 Mngr Locate:            3680ns 1/1/0
 Mngr Resolve:           1633ns 2/2/0
 Rgr Locate Master:      201359ns 1/0/0
 Rgr Locate Meta:        111957ns 1/0/0
 Rgr Data:               99451ns 1/0/0

SWC-DB(client)>
```


##### SELECT cells
```bash
SWC-DB(client)> select where col(FirstColumn)=( cells=() ) DISPLAY_STATS DISPLAY_TIMESTAMP DISPLAY_DATETIME;
2021/09/04 09:00:52.380345269   1630746052380345269     ["My","First","Super","Wide","Column","Key","Fractions"]        The Cell Data Value


Statistics:
 Total Time Took:        700.735 microseconds
 Total Cells Count:      1
 Total Cells Size:       72 B
 Average Transfer Rate:  0.102749 B/microsecond
 Average Cells Rate:     0.00142707 cell/microsecond
 Mngr Locate:            106893ns 2/1/1
 Mngr Resolve:           1674ns 2/2/0
 Rgr Locate Master:      260523ns 2/0/1
 Rgr Locate Meta:        186388ns 2/0/1
 Rgr Data:               94666ns 1/0/0

SWC-DB(client)>
```

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
  add column        add column|schema (schema definitions [name=value ]);
  modify column     modify column|schema (schema definitions [name=value ]);
  delete column     delete column|schema (schema definitions [name=value ]);
  list columns      list|get column|s [OUTPUT_FLAGS] [(NAME|ID)|Comp'expr'..];
                    * OUTPUT_FLAGS: OUTPUT_ONLY_CID
  compact column    compact column|s [(NAME|ID)|Comp'expr',..];
  select            select where [Columns[Cells[Interval Flags]]] Flags DisplayFlags;
                    -> select where COL(NAME|ID|Comp'expr',)=(cells=(Interval Flags)) AND
                         COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(
                           [F-begin] <= range <= [F-end]                   AND
                           [[COMP 'F-start'] <=  key  <= [COMP 'F-finish'] AND]
                           'TS-begin' <= timestamp <= 'TS-finish'          AND
                           offset_key = [F] offset_rev='TS'                AND
                           value COMP 'DATA'
                           LIMIT=NUM   OFFSET=NUM  ONLY_KEYS   ONLY_DELETES     )
                         ) DISPLAY_* TIMESTAMP, DATETIME, SPECS, STATS, BINARY, COLUMN;
                    * DATA-value: PLAN, COUNTER, SERIAL([ID:TYPE:COMP "VALUE", ..])
  update            update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE, ENC), CELL(..) ;
                    -> UPDATE
                         cell(DELETE,                  CID, ['K','E','Y']             );
                         cell(DELETE_VERSION,          CID, ['K','E','Y'], TS         );
                         cell(DELETE_FRACTION,         CID, ['K','E','Y']             );
                         cell(DELETE_FRACTION_VERSION, CID, ['K','E'],     TS         );
                         cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, ''),
                         cell(INSERT,                  CID, ['K','E','Y'], DESC       ),
                         cell(INSERT,                 NAME, ['K','E','Y'], '', 'DATA' ),
                         cell(INSERT_FRACTION,        NAME, ['K','E'],     '', 'DATA' );
                    * FLAG: INSERT|1 DELETE|2 DELETE_VERSION|3
                           INSERT_FRACTION|4 DELETE_FRACTION|5 DELETE_FRACTION_VERSION|6
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
Schema(cid=10 name="FirstColumn" seq=LEXIC type=PLAIN revision=1610393297905674262 compact=0 cell_versions=1 cell_ttl=0 blk_encoding=DEFAULT blk_size=0 blk_cells=0 cs_replication=0 cs_size=0 cs_max=0 log_rollout=0 log_compact=0 log_preload=0)
SWC-DB(client)>
```


##### INSERT Your First SWC-DB cell
```bash
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

SWC-DB(client)>
```


##### SELECT cells
```bash
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

SWC-DB(client)>
```

---
title: SQL
sort: 2
---


# Using SWC-DB SQL



## Syntax Structure of Forms/Types

### The Schema Syntax
The Schema Definition is a word-separator set of key=value in round-brackets ```(key=value )``` . \
The key fields:

| Key           | Type              | Default Value                       | Description                                                       |
| ---           | ---               | ---                                 | ---                                                               |
|cid            |```i64```          | NO_CID == 0                         | the Column ID                                                     |
|name           |```string```       | empty                               | The Column Name                                                   |
|seq            |```string```       | VOLUME                              | The Column Key Sequence, options LEXIC/VOLUME/FC_LEXIC/FC_VOLUME  |
|type           |```string```       | PLAIN                               | The Column Value Type, options PLAIN/COUNTER_(I64/I32/I16/I8)     |
|cell_versions  |```i32```          | 0 == 1                              | The Cell Versions                                                 |
|cell_ttl       |```i32```          | 0 == without                        | The Time to Live in milliseconds                                  |
|blk_encoding   |```string```       | DEFAULT = Rangers' default cfg      | The Block Encoding, options PLAIN/ZSTD/ZLIB/SNAPPY                |
|blk_size       |```i32```          | 0 == Rangers' default cfg           | The Block Size in Bytes                                           |
|blk_cells      |```i32```          | 0 == Rangers' default cfg           | The Number of Cells in a Block                                    |
|cs_replication |```i8```           | 0 == Rangers' default cfg           | The CellStore file Replication                                    |
|cs_size        |```i32```          | 0 == Rangers' default cfg           | The CellStore file size in Byte                                   |
|cs_max         |```i8```           | 0 == Rangers' default cfg           | The Max CellStores in a Range                                     |
|log_rollout    |```i8```           | 0 == Rangers' default cfg           | The ratio of CommitLog-size on reached Write Fragment File        |
|compact        |```i8```           | 0 == Rangers' default cfg           | The Compaction percentage Threshold for doing a range compaction  |
|revision       |```i64```          | 0 == auto assigned on update/create | The Schema's revision                                             |



### Comparators syntax

| TOKEN syntax | Logic Syntax         | Description           |
| ---          | ---                  | ---                   |
|```PF```      | ``` =^ ```           | starts-with [prefix]  |
|```GT```      | ``` >  ```           | greater-than          |
|```GE```      | ``` >= ```           | greater-equal         |
|```EQ```      | ``` == ``` ``` = ``` | equal                 |
|```LE```      | ``` <= ```           | lower-equal           |
|```LT```      | ``` < ```            | lower-than            |
|```NE```      | ``` != ```           | not-equal             |
|```RE```      | ``` r ```            | regular-expression    |
|```VGT```     | ``` v> ```           | volume greater-than   |
|```VGE```     | ``` v>= ```          | volume greater-equal  |
|```VLE```     | ``` v<= ```          | volume lower-equal    |
|```VLT```     | ``` v< ```           | volume lower-than     |



### Column Selector Syntax
The Column Selector Syntax is comma-separated value (name, cid, and a Comparator-Expresssion) such as ```nameOne, 2,r'test$'``` with or without word-separators or a single column by name or cid.



***



## The Available Commands

* Columns Management Commands :
  * [**Create Column**](#createmodifyremove-columns)     - _[```Add``` / ```Create```] ``` ``` [```Column``` / ```Schema```]_ - Create a new Column.
  * [**Modify Column**](#createmodifyremove-columns)     - _[```Modify``` / ```Update```] ``` ``` [```Column``` / ```Schema```]_ - Modify an exting Column Schema.
  * [**Remove Column**](#createmodifyremove-columns)     - _[```Remove``` / ```Delete```] ``` ``` [```Column``` / ```Schema```]_ - Remove a Column.
  * [**Get Columns**](#get-columns)       - _[```Get``` / ```List```] ``` ``` [```Column/s``` / ```Schema/s```]/_ - List all or the requested columns.
  * [**Compact Columns**](#compact-columns)   - _```Compact``` ``` ``` [```Column/s``` / ```Schema/s```]_ - Compact all or the requested columns.

* Data Commands:
  * [**Select [where_clause]**](#select-query) - A Query command to scan and select cells.
  
  * [**Update [cell]**](#update-query) - A Query command to update (insert/delete) cells.



***



## The Commands:

### Create/Modify/Remove Columns
> These commands have aliases for Verb and for Noun:
> * ```create``` == ```add```
> * ```modify``` == ```update```
> * ```remove``` == ```delete```
> * ```remove``` == ```delete```
> * ```column``` == ```schema```

The Syntax of the **Create/Modify/Remove Column** command [```The Command```] [```(```[The Schema Definition](#the-schema-syntax) ```)```]

On Error, the command returns/throw the associated Exception.

_the minimal required definitons_:
* **Create** Column requires column-name ```name=theColumnName```
* **Modify** Column requires column-id and column-name ```cid=CID name=theNewOrExistingName```
* **Remove** Column requires column-name and column-id ```name=theColumnName cid=CID```

#### _a ```create column``` example:_
```
create column(
  name="anExampleColumnName" 
  seq=FC_VOLUME 
  type=PLAIN
)
```

#### _a ```modify column``` example:_
```
modify schema(
  cid=A-CID
  name="anExampleColumnName" 
  seq=VOLUME 
  type=PLAIN 
  revision=input/auto-assigned 
  compact=0 
  cell_versions=1 
  cell_ttl=0 
  blk_encoding=ZSTD
  blk_size=0 
  blk_cells=0 
  cs_replication=0 
  cs_size=0 
  cs_max=0 
  log_rollout=0
)
```

#### _a ```remove column``` example:_
```
remove column(
  cid=AN-ID
  name="anExampleColumnName"
)
```


### Get Columns
> The command has aliases for Verb and for Noun:
> * ```get``` == ```list```
> * ```column``` == ```schema```
> * ```columns``` == ```schemas```

The Syntax of the **Get Column** command [```The Command```] and, optionally in round-brackes ```()``` a [```Column Selector Syntax```](#column-selector-syntax).
> _COMPARATOR'expr' is not applicable for ```SYS_``` name-type columns._ \
> The response is All columns if the _Column Selector_ is not applied.

On Error, the command returns/throw the associated Exception.

#### _a ```get columns``` example:_
```
get columns 1,SYS_MASTER_VOLUME, =^test
```
> The expected response will be schemas of column-cids 1, 2 and the ones matching on `test` prefix



### Compact Columns
> The command has aliases for Noun:
> * ```columns``` == ```schemas```

The Syntax of the **Compact Columns** command [```The Command```] and, optionally in round-brackes ```()``` [```Column Selector Syntax```](#column-selector-syntax) exclusing the Comparator-Expression.
> If the _Column Selector_ is not applied, the request is to compact all columns.

#### _a ```compact columns``` example:_
```
get compact 1,SYS_MASTER_VOLUME
```
> The expected response will be a list of Compact-Results (cid=1 err=CODE) and (cid=2 err=CODE)






### Select Query
The Select Query command perform scans and select cells by the applied query.
* _An Example:_
```
select where
  col(ColNameA1) = (
    cells = (
      range >= ['1-'] AND [>='1-'] <= key = [<='1-1-', ="1" ] AND  value = "Value-Data-1" 
      AND timestamp > “2010/05/29" AND offset_key = ["1-0"] AND offset_rev = 000111222
      limit=10 max_versions=2
    )
  )
  AND col(ColNameB1, ColNameB2) = (
    cells = ( [>='2-'] <= key = [<='2-2-',"1"] AND value = "Value-Data-2"  AND timestamp > "2010/05/29" )
    AND cells = ( key = [<='21-',"1"] AND timestamp > "2010/05/29" )
  )
  max_versions=1
```

* #### The Select Query syntax
***```select where``` [[```Columns-Intervals```](#the-columns-intervals-syntax)] [[```Flags```(global-scope)](#the-flags-syntax)]***


* ##### The Columns-Intervals syntax
The Columns-Intervals is a grouping of Column-Intervals with the TOKEN ``` AND ```, if several Column-Intervals are required. \
***[[```Column-Intervals```](#the-column-intervals-syntax)]*** ***[``` AND ```]*** ***[[```Column-Intervals```](#the-column-intervals-syntax)]***


* ##### The Column-Intervals syntax
***```col(```[```Column Selector Syntax```](#column-selector-syntax) ```) = (``` [[```Cells-Intervals```](#the-cells-intervals-syntax)] ```)```***


* ##### The Cells-Intervals syntax
The Cells-Intervals is a grouping of Cells-Interval with the TOKEN ``` AND ```, if several Cells-Interval are required. \
***[[```Cells-Interval```](#the-cells-interval-syntax)]*** ***[``` AND ```]*** ***[[```Cells-Interval```](#the-cells-interval-syntax)]***


* ##### The Cells-Interval syntax
The Cells-Interval is a group of conditions, joined by the TOKEN ``` AND ``` plus Flags, for matching cells against it. \
All the Conditions and Flags are optional, without a LIMIT and any Conditions the condition is equal to select all cells in the column/s . \
***```cells=(```
[ [```Condition-Range```](#the-condition-range-syntax) ]
[``` AND ```]
[ [```Condition-Key-Interval```](#the-condition-key-interval-syntax) ]
[``` AND ```]
[ [```Condition-Value```](#the-condition-value-syntax) ]
[``` AND ```]
[ [```Condition-Timestamp```](#the-condition-timestamp-syntax) ]
[``` AND ```]
[ [```Condition-Offset-Key```](#the-condition-offset-key-syntax) ]
[``` AND ```]
[ [```Condition-Offset-Revision```](#the-condition-offset-revision-syntax) ]
[ [```Flags```(cells-interval-scope)](#the-flags-syntax)] 
```)```***


* ##### The Condition-Range syntax
The Condition of Range is an interval of from Key to Key, optionally to apply only one side with Key on the right side. It can be defined only by the Comparators GE and LE. \
***```Key``` [``` <= ```] ``` range ``` [``` <= ```] ```Key```***


* ##### The Condition-Key-Interval syntax
The Condition of Key Interval is an interval of from Condition-Key to Condition-Key, optionally to apply only one side with Condition-Key on the right side. It can be defined only by the Comparators EQ, GE and LE. \
The Exact Cell Key match condition is when Comparator is EQ and Condition-Key is set with EQ on all the Fractions. \
***[ [```Condition-Key```](#the-condition-key-syntax)] 
[``` <= ```] ``` key ``` [``` <= ```] 
[ [```Condition-Key```](#the-condition-key-syntax)]***


* ##### The Condition-Key syntax
The Condition of Key is the Fractions in square-brackets with each Fraction having an option to match on Comparator with Value/Expression. Without a Comparator applied to a Fraction the Comparator is auto-set to EQ. \
Optionally to select all the inner/deeper-level fractions including or excluding the prior Fraction by setting last fraction with Comparators GT or GE. \
***```[ COMP"F1",  COMP"F2",  COMP"F3",  COMP"F4",  COMP"F5",  COMP"F6",  COMP"F7", >="" ]```***


* ##### The Condition-Value syntax
The Condition of Value. The Comparator is auto-set to EQ, If no Comparator was applied. \
***``` value COMP "EXPR/VALUE " ```***


* ##### The Condition-Timestamp syntax
The Condition of Timestamp is an interval of from Timestamp to Timestamp, optionally to apply only one side with VALUE on right. It can be defined only by the Comparators NE, EQ, GT, LT, GE and LE. The VALUE can be a Timestamp in nanoseconds or a Date and Time in format ```"YYYY/MM/DDD HH:mm:ss.nanoseconds"```.  \
***```VALUE``` [``` COMP ```] ``` timestamp ``` [``` COMP ```] ```VALUE```***


* ##### The Condition-Offset-Key syntax
The Condition of the Offset Key is a Key with one Comparator option Equal. The Cell of the Key will be evaluated for select-match if Offset Revision was not specified. \
***```offset_key``` ``` = ``` ```Key```***


* ##### The Condition-Offset-Revision syntax
The Condition of the Offset Revision is the Cell Timestamp with one Comparator option Equal. The Timestamp is in Nanoseconds or Date & Time, the ```offset_rev``` is the timestamp of the last cell that will be skipped before evaluating for select-match. \
***```offset_rev``` ``` = ``` ```Timestamp```***


* ##### The Flags syntax
The following flags, ```token``` and ```key=value```, are available: \
```LIMIT=I64``` ```OFFSET=I64``` ```MAX_VERSIONS=I64``` ```MAX_BUFFER=I64``` ```ONLY_KEYS```   ```ONLY_DELETES```.
* On global-scope the flags applied to all Cells-Interval without a Cells-Interval-scope flags.
* On Cells-Interval-scope the flags applied only to the Cells-Interval scope.








### Update Query
The Update Query command performs cells write to the designated range/s.

* _An Example:_
```
UPDATE
  cell(DELETE,                  CID, ['K','E','Y']             ),
  cell(DELETE_VERSION,          CID, ['K','E','Y'], TS         ),
  cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, 'DATA' ),
  cell(INSERT,                  CID, ['K','E','Y'], DESC, 'DATA' ),
  cell(INSERT,                 NAME, ['K','E','Y'], '', 'DATA' )
```


* #### The Update Query syntax
The Update Query sytrax consists the 'update' command followed by Cell or comma-separated Cell/s
``` UPDATE ``` [```Cell```](#the-cell-for-update-syntax) ```,``` [```Cell```](#the-cell-for-update-syntax)


* ##### The Cell for Update syntax
The Syntax by Flags and Options:
  * **DELETE** \
``` cell( ``` 
``` DELETE ```
```,``` 
``` column ID/NAME``` 
```,``` 
``` Key``` 
``` ) ```

  * **DELETE_VERSION** \
``` cell( ```
``` DELETE_VERSION```
```,``` 
``` column ID/NAME```
```,``` 
``` Key```
```,``` 
``` TIMESTAMP ```
``` ) ```

  * **INSERT** with version-timestamp config \
``` cell( ```
``` INSERT```
```,``` 
``` column ID/NAME```
```,``` 
``` Key```
```,``` 
``` TIME_ORDER,```
``` TIMESTAMP``` 
```,``` 
``` VALUE-DATA ```
``` ) ```
  > * _TIME_ODER_ options are ```ASC```/```DESC``` - empty-```entry,``` defaults to ```DESC``` .
  > * _TIMESTAMP_ in nanoseconds or a Date and Time in format ```"YYYY/MM/DDD HH:mm:ss.nanoseconds"```, empty-field is auto-assign.

  * **INSERT** with auto-timestamp \
``` cell( ```
``` INSERT```
```,``` 
``` column ID/NAME```
```,``` 
``` Key```
```,``` 
``` "" ``` 
```,``` 
``` VALUE-DATA ```
``` ) ```


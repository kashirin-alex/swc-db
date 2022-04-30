---
title: SQL
sort: 2
---


# Using SWC-DB SQL



## Table of Contents

* [The Syntax Structure of Forms/Types](#syntax-structure-of-formstypes)
  * [The Schema Syntax](#the-schema-syntax)
  * [The Comparators syntax](#the-comparators-syntax)
  * [The Column Selector Syntax](#the-column-selector-syntax)
* [The Available Commands](#the-available-commands)
* [The Commands](#the-available-commands)
  * [Create/Modify/Remove Columns](#createmodifyremove-columns)
  * [Get Columns](#get-columns)
    * [A get columns example](#a-get-columns-example)
  * [Compact Columns](#compact-columns)
    * [A compact columns example](#a-compact-columns-example)
  * [Select Query](#select-query)
    * [The Select Query syntax](#the-select-query-syntax)
    * [The Columns-Intervals syntax](#the-columns-intervals-syntax)
    * [The Column-Intervals syntax](#the-column-intervals-syntax)
    * [The Cells-Interval syntax](#the-cells-interval-syntax)
    * [The Condition-Range syntax](#the-condition-range-syntax)
    * [The Key-Intervals syntax](#the-key-intervals-syntax)
    * [The Condition-Key-Interval syntax](#the-condition-key-interval-syntax)
    * [The Condition-Key syntax](#the-condition-key-syntax)
    * [The Condition-Value syntax](#the-condition-value-syntax)
    * [The Condition-Value-Expression syntax](#the-condition-value-expression-syntax)
    * [The Condition-Timestamp syntax](#the-condition-timestamp-syntax)
    * [The Condition-Offset-Key syntax](#the-condition-offset-key-syntax)
    * [The Condition-Offset-Revision syntax](#the-condition-offset-revision-syntax)
    * [The Flags syntax](#the-flags-syntax)
    * [The Update Options syntax](#the-update-options-syntax)
  * [Update Query](#update-query)
    * [The Update Query syntax](#the-update-query-syntax)
    * [The Cell for Update syntax](#the-cell-for-update-syntax)
    * [A DELETE_LE Flag](#a-delete_le-flag)
    * [A DELETE_EQ Flag](#a-delete_eq-flag)
    * [An INSERT Flag with auto-timestamp](#an-insert-flag-with-auto-timestamp)
    * [An INSERT Flag with version-timestamp config](#an-insert-flag-with-version-timestamp-config)
    * [An INSERT Flag for a SERIAL Column Type](#an-insert-flag-for-a-serial-column-type)
    * [An INSERT Flag with Encoded-Value](#an-insert-flag-with-encoded-value)

*****


## Syntax Structure of Forms/Types

### The Schema Syntax
The Schema Definition is a word-separator set of key=value in round-brackets ```(key=value )``` . \
The key fields:

| Key           | Type              | Default Value                       | Description                                                       |
| ---           | ---               | ---                                 | ---                                                               |
|cid            |```i64```          | NO_CID == 0                         | the Column ID                                                     |
|name           |```string```       | empty                               | The Column Name                                                   |
|tags           |```strings```      | empty == []                         | The Column Tags                                                   |
|seq            |```string```       | VOLUME                              | The Column Key Sequence, options LEXIC / VOLUME / FC_LEXIC / FC_VOLUME  |
|type           |```string```       | PLAIN                               | The Column Value Type, options PLAIN / COUNTER_I{64,32,16,8} / SERIAL     |
|cell_versions  |```i32```          | 0 == 1                              | The Cell Versions                                                 |
|cell_ttl       |```i32```          | 0 == without                        | The Time to Live in seconds                                       |
|blk_encoding   |```string```       | DEFAULT = Rangers' default cfg      | The Block Encoding, options PLAIN / ZSTD / ZLIB / SNAPPY          |
|blk_size       |```i32```          | 0 == Rangers' default cfg           | The Block Size in Bytes                                           |
|blk_cells      |```i32```          | 0 == Rangers' default cfg           | The Number of Cells in a Block                                    |
|cs_replication |```i8```           | 0 == Rangers' default cfg           | The CellStore file Replication                                    |
|cs_size        |```i32```          | 0 == Rangers' default cfg           | The CellStore file size in Byte                                   |
|cs_max         |```i8```           | 0 == Rangers' default cfg           | The Max CellStores in a Range                                     |
|log_rollout    |```i8```           | 0 == Rangers' default cfg           | The ratio of CommitLog-size on reached Write Fragment File        |
|log_compact    |```i8```           | 0 == Rangers' default cfg           | The size of cointervaling Fragments for log compaction to apply   |
|log_preload    |```i8```           | 0 == Rangers' default cfg           | The number of Fragments to be preloaded at scans and compaction   |
|compact        |```i8```           | 0 == Rangers' default cfg           | The Compaction percentage Threshold for doing a range compaction  |
|revision       |```i64```          | 0 == auto assigned on update/create | The Schema's revision                                             |



### The Comparators syntax

| TOKEN syntax | Logic Syntax         | Description               |
| ---          | ---                  | ---                       |
|```PF```      | ``` =^ ```           | starts-with [prefix]      |
|```GT```      | ``` >  ```           | greater-than              |
|```GE```      | ``` >= ```           | greater-equal             |
|```EQ```      | ``` == ``` ``` = ``` | equal                     |
|```LE```      | ``` <= ```           | lower-equal               |
|```LT```      | ``` < ```            | lower-than                |
|```NE```      | ``` != ```           | not-equal                 |
|```RE```      | ``` r ```            | regular-expression        |
|```VGT```     | ``` v> ```           | volume greater-than       |
|```VGE```     | ``` v>= ```          | volume greater-equal      |
|```VLE```     | ``` v<= ```          | volume lower-equal        |
|```VLT```     | ``` v< ```           | volume lower-than         |
|```SBS```     | ``` %> ```           | subset                    |
|```SPS```     | ``` <% ```           | superset                  |
|```POSBS```   | ``` ~> ```           | eq/part ordered subset    |
|```POSPS```   | ``` <~ ```           | eq/part ordered superset  |
|```FOSBS```   | ``` <- ```           | eq/full ordered superset  |
|```FOSPS```   | ``` -> ```           | eq/full ordered superset  |



### The Column Selector Syntax
The Column Selector Syntax is comma-separated values with available select options:
  * cid - an exact match of column cid
  * name - an exact match on column name
  * comparator with name - a column name pattern selector
  * tags - a comparator to square-brackets of tags-list and inner pattern comparator for every tag-string

The following Syntax, to the comma-separated values, rules applied:
  1. space/tab/newline not in-quotes is ignored/skipped.
  2. a digit-value without quotes is evaluated as `cid`.
  3. a string-value without quotes is at first evaluated for Comparator and rest is value (eg. `rrest` is RE with value "rest" whereas if the required is -EQ "rrest" the string should be qouted).
  4. an overall or a reserved, for non-qouted values, token for lookup is the `tags` token and each tag in the tags-list is evaluate by rule-3.

As example, the Selector `nameOne, 2, =^'test', tags%>[2,v>"5",=1]` result will be the columns: a column with column-name "nameOne", a column with cid "2", columns with column-name starts-with(prefix) "test" and columns with tags consist of tags "2","1" and above volumetric "5" and all the tags are being the superset to the subset.

> Only the exact match of cid and name are options to be first probed on the cache of the application's(Clients) Schemas-Cache rest are made with a request to a Schemas Role Manager.



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
  log_compact=0
  log_preload=0
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

The Syntax of the **Get Column** command [```The Command```] and, optionally in round-brackes ```()``` a [```Column Selector Syntax```](#the-column-selector-syntax).
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

The Syntax of the **Compact Columns** command [```The Command```] and, optionally in round-brackes ```()``` [```Column Selector Syntax```](#the-column-selector-syntax).
> If the _Column Selector_ is not applied, the request is to compact all columns.

#### _a ```compact columns``` example:_
```
get compact 1,SYS_MASTER_VOLUME
```
> The expected response will be a list of Compact-Result [(cid=1 err=CODE), (cid=2 err=CODE)]






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
***```col(```[```Column Selector Syntax```](#the-column-selector-syntax) ```) = (``` [[```Cells-Intervals```](#the-cells-intervals-syntax)] ```)```***


* ##### The Cells-Intervals syntax
The Cells-Intervals is a grouping of Cells-Interval with the TOKEN ``` AND ```, if several Cells-Interval are required. \
***[[```Cells-Interval```](#the-cells-interval-syntax)]*** ***[``` AND ```]*** ***[[```Cells-Interval```](#the-cells-interval-syntax)]***


* ##### The Cells-Interval syntax
The Cells-Interval is a group of conditions, joined by the TOKEN ``` AND ``` plus Flags, for matching cells against it. \
All the Conditions, Flags and Options are optional, without a LIMIT and any Conditions the condition is equal to select all cells in the column/s . \
***```cells=(```
[ [```Condition-Range```](#the-condition-range-syntax) ]
[``` AND ```]
[ [```Key-Intervals```](#the-key-intervals-syntax) ]
[``` AND ```]
[ [```Condition-Value```](#the-condition-value-syntax) ]
[``` AND ```]
[ [```Condition-Timestamp```](#the-condition-timestamp-syntax) ]
[``` AND ```]
[ [```Condition-Offset-Key```](#the-condition-offset-key-syntax) ]
[``` AND ```]
[ [```Condition-Offset-Revision```](#the-condition-offset-revision-syntax) ]
[ [```Flags```(cells-interval-scope)](#the-flags-syntax)]
[ [```Options```](#the-update-options-syntax)]
```)```***


* ##### The Condition-Range syntax
The Condition of Range is an interval of from Key to Key, optionally to apply only one side with Key on the right side. It can be defined only by the Comparators GE and LE. \
***```Key``` [``` <= ```] ``` range ``` [``` <= ```] ```Key```***


* ##### The Key-Intervals syntax
The Key-Intervals are several or single ```Condition-Key-Interval```, joined by the TOKEN ``` AND ```.
Whereas the given order is the matching order which let further matching of a cell-key,
such as 1st(```Condition-Key-Interval```) is based on a sequential Comparators
while the 2nd and followed(```Condition-Key-Interval```) include/involve non-sequential Comparators such as Regexp. \
The 1st ```Condition-Key-Interval``` is the main(after ```Condition-Range```) Interval used by/for locators of Range & Range-Blocks. \
***[ [```Condition-Key-Interval```](#the-condition-key-interval-syntax) ]
```...```
[``` AND ```]
[ [```Condition-Key-Interval```](#the-condition-key-interval-syntax) ]
[``` AND ```]
[ [```Condition-Key-Interval```](#the-condition-key-interval-syntax) ]***



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
The Condition of Value, a single or several Value Conditions joined by an ```AND``` TOKEN. The Comparator is auto-set to EQ, If no Comparator was applied. \
***``` value ``` [```Condition-Value-Expression```](#the-condition-value-expression-syntax)```...```
[``` AND ```]
``` value ``` [```Condition-Value-Expression```](#the-condition-value-expression-syntax)
[``` AND ```]
``` value ``` [```Condition-Value-Expression```](#the-condition-value-expression-syntax)***


* ##### The Condition-Value-Expression syntax
The Expression of Value Condition dependable on the [Schema's column value type](#the-schema-syntax).
  * **_PLAIN_**: \
  ``` COMP "VALUE" ``` - applicable with Extended Comparators

  * **_COUNTER_**: \
  ``` COMP "VALUE" ``` - not supported Comparators PF, RE, POSBS and POSPS

  * **_SERIAL_**: \
  ``` [ID:TYPE:COMP "VALUE", ... ] ``` - in square-brackets a comma-separated sets, a set is separated by colon with Field-ID, Field-Type and a Comparator with a Value. \
  The applicable Comparators depend on the Field-Type: ```BYTES(B)``` as PLAIN, ```INT64(I)```/```DOUBLE(D)``` as COUNTER, ```KEY(K)``` with KeySeq(LEXIC/VOLUME) followed by a [Condition-Key](#the-condition-key-syntax) and in list-syntax ```COMP[COMP VALUE, .. ]``` ```LIST_INT64(LI)``` Value as COUNTER and ```LIST_BYTES(LB)``` Value as PLAIN. \
  The SERIAL match requires all field-definitions matching ID+TYPE+COND, whereas Field-ID can have multiple Field-Type and Value definitions.\
  _A data-set Example_, a cell-value: \
    ``` TS KEY  [0:I:1, 1:I:5, 2:I:1, 3:D:1.0, 4:B:"aBcdef", 5:K:[abc,def,ghi,4,5], 6:LI:[1,2,3,4,5,6,7], 7:LB:[abc,def], 8:B:"More-Bytes] ``` \
    can have the following Condition-Value syntax: \
    ``` value == [0:I:==1, 1:I:>4, 2:I:<5, 3:D:>0.123456, 4:B:=^aBc, 5:K:VOLUME[abc,def,ghi,>=""], 6:LI:<=[1,2,3,>0,5,6,==7,0], 7:LB:%>[~>ef,~>ac] ] ```
    * In this case Field with ID=8 of BYTES type does not require the expression match.


* ##### The Condition-Timestamp syntax
The Condition of Timestamp is an interval of from Timestamp to Timestamp, optionally to apply only one side with VALUE on right. It can be defined only by the Comparators NE, EQ, GT, LT, GE and LE. The VALUE can be a Timestamp in nanoseconds or a Date and Time in format ```"YYYY/MM/DD HH:mm:ss.nanoseconds"```.  \
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


* ##### The Update Options syntax
The Select Query can be applied with ```DELETE_MATCHING``` or ```UPDATE [OP] (TIMESTAMP, VALUE, ENC)``` options for updating a cells-interval.
  * The supported UPDATE operations(`OP`)  :

    | syntax | name                   | supported column-types        | description                                                                                                                                  |
    |  ---   |   ---                  |       ---                     |     ---                                                                                                                                      |
    |  `=`   | REPLACE                | PLAIN, SERIAL, COUNTER        | replace with the update value (_default as well if other OP not supported by the col-type_)                                                  |
    |  `+=`  | APPEND                 | PLAIN, SERIAL                 | appends the update value to the cell's current value                                                                                         |
    |  `=+`  | PREPEND                | PLAIN, SERIAL                 | prepends the update value to the cell's current value                                                                                        |
    |  `+:#` | INSERT                 | PLAIN                         | inserts the update value at position in current value (appends if pos above value)                                                            |
    |  `=:#` | OVERWRITE              | PLAIN                         | overwrites the current value at position with new value (appends if pos above value)                                                          |
    |  `~=`  | SERIAL                 | SERIAL                        | update is done by the inner serial-fields defintions (```UPDATE[OP](TS,[ID:TYPE:[OP]val, ..])```)                                            |

  * The supported UPDATE operations(`OP`) for numberic with MATH-op in the inner SERIAL fields:

    | syntax | name                   | supported field-types         | description                                                                                                                                  |
    |  ---   |   ---                  |       ---                     |     ---                                                                                                                                      |
    |  `=`   | EQUAL                  | INT64, DOUBLE                 | set field value to the new value (_default_)                                                                                                 |
    |  `+=`  | PLUS                   | INT64, DOUBLE                 | plus new value to field's value (negative number allowed)                                                                                    |
    |  `*=`  | MULTIPLY               | INT64, DOUBLE                 | multiply current value by update value                                                                                                       |
    |  `/=`  | DIVIDE                 | INT64, DOUBLE                 | divide current value by the new value (ignored at zero)                                                                                      |
    |  `!`   | CTRL_NO_ADD_FIELD      | INT64, DOUBLE                 | Applicable infront of other OPs. A control operation, in case a field for update does not exist, to not add a new field                      |
    |  `DEL` | CTRL_DELETE_FIELD      | INT64, DOUBLE                 | delete the given field (without value after OP)                                                                                              |
    |  `I`   | CTRL_VALUE_SET         | INT64, DOUBLE                 | add/set if not exists (only available with used by in List at BY_UNIQUE OR BY_COND)                                                          |
    |  `O`   | CTRL_VALUE_DEL         | INT64, DOUBLE                 | delete any that exist (only available with used by in List at BY_UNIQUE OR BY_COND)                                                          |

  * The supported UPDATE operations(`OP`) for array/list/bytes with LIST-op in the inner SERIAL fields:

    | syntax | name                   | supported field-types         | description                                                                                                                                  |
    |  ---   |   ---                  |       ---                     |     ---                                                                                                                                      |
    |  `=`   | REPLACE                | BYTES, LIST_BYTES, LIST_INT64 | replace with the update value (_default_)                                                                                                    |
    |  `+=`  | APPEND                 | BYTES, LIST_BYTES, LIST_INT64 | appends the update value to a field value                                                                                                    |
    |  `=+`  | PREPEND                | BYTES, LIST_BYTES, LIST_INT64 | prepends the update value to a field value                                                                                                   |
    |  `+:#` | INSERT                 | BYTES, LIST_BYTES, LIST_INT64 | insert the update value at position in a field value (appends if pos above value)                                                            |
    |  `=:#` | OVERWRITE              | BYTES, LIST_BYTES, LIST_INT64 | overwrite a field value at position with new value (appends if pos above value)                                                              |
    |  `-:#` | ERASE                  | BYTES, LIST_BYTES, LIST_INT64 | erases the position in a field value                                                                                                         |
    |  `!`   | CTRL_NO_ADD_FIELD      | BYTES, LIST_BYTES, LIST_INT64 | except for BY_INDEX, Applicable infront of other OPs, a control operation, in case a field for update does not exist, to not add a new field |
    |  `DEL` | CTRL_DELETE_FIELD      | BYTES, LIST_BYTES, LIST_INT64 | delete the given field (without value after OP)                                                                                              |
    |  `I`   | CTRL_VALUE_SET         | BYTES                         | add/set if not exists (only available with used by in List at BY_UNIQUE OR BY_COND)                                                          |
    |  `O`   | CTRL_VALUE_DEL         | BYTES                         | delete any that exist (only available with used by in List at BY_UNIQUE OR BY_COND)                                                          |
    |  `U~`  | BY_UNIQUE              | LIST_BYTES, LIST_INT64        | the field value items have CTRL_VALUE_SET/DEL OP                                                                                             |
    |  `C~`  | BY_COND                | LIST_BYTES, LIST_INT64        | the field value items have CTRL_VALUE_SET/DEL OP and [COMP] (UPDATE~=(TS,[ID:LI:`C~`[[OP]`[COMP]`val, [OP]`[COMP]`val, ..] ]))               |
    |  `?:`  | BY_INDEX               | LIST_BYTES, LIST_INT64        | the field value is with IDX & OP in items (UPDATE~=(TS,```[ID:LI:?:[ [IDX][OP]val, [IDX][OP]val, ..] ]```))                                  |

The cells of the response are the fetched cells before update has been applied. \
These options let the use of a column in a synchronized manner that open usage possibilities to cases such as:
  * auto-increment value for purpose such as an unique ID.
  * the data to be considered and processed as a Queue, whether to delete or to update the value with a corresponding queue track data.






### Update Query
The Update Query command performs cells write to the column/s and the designated range/s.

* #### The Update Query syntax
The Update Query sytrax consists the 'update' command followed by Cell or comma-separated Cell/s
``` UPDATE ``` [```Cell```](#the-cell-for-update-syntax) ```,``` [```Cell```](#the-cell-for-update-syntax)
* _An Example:_
```
UPDATE
  cell(DELETE_LE,               CID, ['K','E','Y']                         ),
  cell(DELETE_EQ,               CID, ['K','E','Y'], TS                     ),
  cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, 'DATA'        ),
  cell(INSERT,                  CID, ['K','E','Y'], DESC,    'DATA'        ),
  cell(INSERT,                 NAME, ['K','E','Y'], '',      'DATA', 'ENC' );
```


* #### The Cell for Update syntax
The Syntax depends on the Flags and available definitions.

* ##### A **DELETE_LE** Flag:
  ```cell(``` ``` DELETE_LE ``` ```,``` ``` Column ID|NAME ``` ```,``` ``` Key ``` ```) ```

* ##### A **DELETE_EQ** Flag:
  ```cell(``` ``` DELETE_EQ ``` ```,``` ``` Column ID|NAME ``` ```,``` ``` Key ``` ```,``` ``` TIMESTAMP ``` ```) ```

* ##### An **INSERT** Flag with auto-timestamp:
  ```cell(``` ``` INSERT ``` ```,``` ``` Column ID|NAME ``` ```,``` ``` Key ``` ```,``` ``` "" ``` ```,``` ``` VALUE-DATA ``` ```) ```

* ##### An **INSERT** Flag with version-timestamp config:
  ```cell(``` ``` INSERT ``` ```,``` ``` Column ID|NAME ``` ```,``` ``` Key ``` ```,``` ``` TIME_ORDER ``` ```,``` ``` TIMESTAMP ``` ```,``` ``` VALUE-DATA ``` ```) ```
  > * _TIME_ODER_ options are ```ASC```/```DESC``` - empty-```entry,``` defaults to ```DESC``` .
  > * _TIMESTAMP_ in nanoseconds or a Date and Time in format ```"YYYY/MM/DD HH:mm:ss.nanoseconds"```, empty-field is auto-assign.

* ##### An **INSERT** Flag for a SERIAL Column Type:
  As other **INSERT** with or without timestamp whereas ``` VALUE-DATA ``` defined in a serialization format. \
  The ``` VALUE-DATA ``` is a square-brackets of a field-sets ```[ID:TYPE:VALUE, ... ]```. The field-sets are unordered makes the less accessed field to be preferred as last.\
  An expectation of Field ID is to be an unique ID per each type, It is OK to have ```[1:I:123, 1:D:0.123, 1:B:"123", 1:K:[1,2,3]], 1:LI:[1,2,3], 1:LB:[ab,cd,ef]```
  * The Field-ID is a UINT24_T - max 16,777,215 possible Field-IDs.
  * The Available Field-Type: | ```INT64``` / ```I``` | ```DOUBLE``` / ```D``` | ```BYTES``` / ```B``` | ```KEY``` / ```K``` | ```LIST_INT64``` / ```LI``` | ```LIST_BYTES``` / ```LB``` |

* ##### An **INSERT** Flag with Encoded-Value:
  The last definition of cell is set with the ENCODER.\
  ```cell(``` ``` INSERT``` ```,``` ``` Column ID/NAME``` ```,``` ``` Key ``` ```,``` ``` "" ``` ```,``` ``` VALUE-DATA ``` ```,``` ``` ENCODER ``` ```) ```
  * The available encoding for cell-value: ```ZLIB```, ```SNAPPY```, ```ZSTD``` - default without.



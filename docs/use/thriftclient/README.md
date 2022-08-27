---
title: Thrift Client
sort: 3
---


# The SWC-DB Thrift Protocol & Supported Languages


## _The Supported Languages by SWC-DB Thrift_

 _**Available SWC-DB Thrift Client Service libraries**_:

  * [C++](cpp/)
  * [Java](java/)
  * [Python](python/)
  * [Ruby](ruby/)
  * [C-Glib](c_glib/)

 _**Additional only generated SWC-DB Thrift Client Service**_:

  * [Netstd](netstd/)
  * [Rust](rust/)


> Languages Support can be extended upto [supported languages by Apache-Thrift ](https://github.com/apache/thrift/blob/master/LANGUAGES.md)


_**Some of use cases can be found at**_:
  * [Examples](https://github.com/kashirin-alex/swc-db/tree/master/examples)
  * [Tests](https://github.com/kashirin-alex/swc-db/tree/master/tests/integration/thrift)


***



## _The SWC-DB Thrift Modules_




| Module | Services & Functions | Data types | Constants |
| --- | --- | --- | --- |
|Service|[Service](#service-service)|[KeySeq](#enumeration-keyseq)|[TIMESTAMP_NULL](#constant-timestamp_null)|
||	[ &bull; sql_mng_column](#function-servicesql_mng_column)|[ColumnType](#enumeration-columntype)|[TIMESTAMP_AUTO](#constant-timestamp_auto)|
||	[ &bull; sql_list_columns](#function-servicesql_list_columns)|[EncodingType](#enumeration-encodingtype)|[FU_CTRL_DEFAULT](#constant-fu_ctrl_default)|
||	[ &bull; sql_compact_columns](#function-servicesql_compact_columns)|[SchemaFunc](#enumeration-schemafunc)|[FU_CTRL_NO_ADD_FIELD](#constant-fu_ctrl_no_add_field)|
||	[ &bull; sql_select](#function-servicesql_select)|[Comp](#enumeration-comp)|[FU_CTRL_DELETE_FIELD](#constant-fu_ctrl_delete_field)|
||	[ &bull; sql_select_rslt_on_column](#function-servicesql_select_rslt_on_column)|[SpecFlagsOpt](#enumeration-specflagsopt)|[FU_CTRL_VALUE_SET](#constant-fu_ctrl_value_set)|
||	[ &bull; sql_select_rslt_on_key](#function-servicesql_select_rslt_on_key)|[SpecIntervalOptions](#enumeration-specintervaloptions)|[FU_CTRL_VALUE_DEL](#constant-fu_ctrl_value_del)|
||	[ &bull; sql_select_rslt_on_fraction](#function-servicesql_select_rslt_on_fraction)|[UpdateOP](#enumeration-updateop)||
||	[ &bull; sql_query](#function-servicesql_query)|[Flag](#enumeration-flag)||
||	[ &bull; sql_update](#function-servicesql_update)|[FU_MATH_OP](#enumeration-fu_math_op)||
||	[ &bull; exec_sql](#function-serviceexec_sql)|[FU_LIST_OP](#enumeration-fu_list_op)||
||	[ &bull; updater_create](#function-serviceupdater_create)|[CellsResult](#enumeration-cellsresult)||
||	[ &bull; updater_close](#function-serviceupdater_close)|[Schemas](#typedef-schemas)||
||	[ &bull; update](#function-serviceupdate)|[Key](#typedef-key)||
||	[ &bull; update_serial](#function-serviceupdate_serial)|[SpecKey](#typedef-speckey)||
||	[ &bull; update_by_types](#function-serviceupdate_by_types)|[SpecKeyIntervals](#typedef-speckeyintervals)||
||	[ &bull; mng_column](#function-servicemng_column)|[SpecValues](#typedef-specvalues)||
||	[ &bull; list_columns](#function-servicelist_columns)|[SpecValueSerialFields](#typedef-specvalueserialfields)||
||	[ &bull; compact_columns](#function-servicecompact_columns)|[SpecValuesSerial](#typedef-specvaluesserial)||
||	[ &bull; scan](#function-servicescan)|[UCells](#typedef-ucells)||
||	[ &bull; scan_rslt_on_column](#function-servicescan_rslt_on_column)|[UCCells](#typedef-uccells)||
||	[ &bull; scan_rslt_on_key](#function-servicescan_rslt_on_key)|[CellValuesSerial](#typedef-cellvaluesserial)||
||	[ &bull; scan_rslt_on_fraction](#function-servicescan_rslt_on_fraction)|[CellValuesSerialOp](#typedef-cellvaluesserialop)||
||	[ &bull; scan_rslt_on](#function-servicescan_rslt_on)|[UCellsSerial](#typedef-ucellsserial)||
|||[UCCellsSerial](#typedef-uccellsserial)||
|||[CCells](#typedef-ccells)||
|||[KCells](#typedef-kcells)||
|||[CompactResults](#typedef-compactresults)||
|||[Exception](#exception-exception)||
|||[Schema](#struct-schema)||
|||[SchemaPattern](#struct-schemapattern)||
|||[SchemaTagsPatterns](#struct-schematagspatterns)||
|||[SchemaPatterns](#struct-schemapatterns)||
|||[SpecSchemas](#struct-specschemas)||
|||[SpecFlags](#struct-specflags)||
|||[SpecFraction](#struct-specfraction)||
|||[SpecTimestamp](#struct-spectimestamp)||
|||[SpecKeyInterval](#struct-speckeyinterval)||
|||[SpecValue](#struct-specvalue)||
|||[SpecUpdateOP](#struct-specupdateop)||
|||[SpecIntervalUpdate](#struct-specintervalupdate)||
|||[SpecIntervalUpdateSerial](#struct-specintervalupdateserial)||
|||[SpecInterval](#struct-specinterval)||
|||[SpecColumn](#struct-speccolumn)||
|||[SpecValueSerial_INT64](#struct-specvalueserial_int64)||
|||[SpecValueSerial_DOUBLE](#struct-specvalueserial_double)||
|||[SpecValueSerial_BYTES](#struct-specvalueserial_bytes)||
|||[SpecValueSerial_KEY](#struct-specvalueserial_key)||
|||[SpecValueSerial_LI](#struct-specvalueserial_li)||
|||[SpecValueSerial_LB](#struct-specvalueserial_lb)||
|||[SpecValueSerialField](#struct-specvalueserialfield)||
|||[SpecValueSerial](#struct-specvalueserial)||
|||[SpecIntervalSerial](#struct-specintervalserial)||
|||[SpecColumnSerial](#struct-speccolumnserial)||
|||[SpecScan](#struct-specscan)||
|||[UCell](#struct-ucell)||
|||[CellValueSerial](#struct-cellvalueserial)||
|||[FU_INT64](#struct-fu_int64)||
|||[FU_DOUBLE](#struct-fu_double)||
|||[FU_BYTES](#struct-fu_bytes)||
|||[FU_LI](#struct-fu_li)||
|||[FU_LB](#struct-fu_lb)||
|||[CellValueSerialOp](#struct-cellvalueserialop)||
|||[UCellSerial](#struct-ucellserial)||
|||[Cell](#struct-cell)||
|||[CellSerial](#struct-cellserial)||
|||[Cells](#struct-cells)||
|||[CCell](#struct-ccell)||
|||[CCellSerial](#struct-ccellserial)||
|||[ColCells](#struct-colcells)||
|||[KCell](#struct-kcell)||
|||[KCellSerial](#struct-kcellserial)||
|||[kCells](#struct-kcells)||
|||[FCell](#struct-fcell)||
|||[FCellSerial](#struct-fcellserial)||
|||[FCells](#struct-fcells)||
|||[CellsGroup](#struct-cellsgroup)||
|||[CompactResult](#struct-compactresult)||
|||[Result](#struct-result)||


***
## Constants

|Constant|Type|Value||
|---|---|---|---|
| ```TIMESTAMP_NULL``` | ```i64```| ``````-9223372036854775807`````` |The TIMESTAMP NULL value |
| ```TIMESTAMP_AUTO``` | ```i64```| ``````-9223372036854775806`````` |The TIMESTAMP AUTO value |
| ```FU_CTRL_DEFAULT``` | ```i8```| ``````0`````` |A control bit of default-state |
| ```FU_CTRL_NO_ADD_FIELD``` | ```i8```| ``````1`````` |A control bit to not add a new field in case a field for update does not exist (Except for BY_INDEX OP) |
| ```FU_CTRL_DELETE_FIELD``` | ```i8```| ``````2`````` |A control bit to delete the given field |
| ```FU_CTRL_VALUE_SET``` | ```i8```| ``````4`````` |A control bit to add/set if not exists (only available with OP used by BY_UNIQUE OR BY_COND in List field-types ) |
| ```FU_CTRL_VALUE_DEL``` | ```i8```| ``````8`````` |A control bit delete any that exist (only available with OP used by BY_UNIQUE OR BY_COND in List field-types ) |

***
## Enumerations

### Enumeration: KeySeq
Column Key Sequences 

|Name|Value|Description|
|---|---|---|
|```UNKNOWN```|```0```|Unknown/Unrecognized Type |
|```LEXIC```|```1```|The Lexical Key Order Sequence |
|```VOLUME```|```2```|The Volumetric Key Order Sequence |
|```FC_LEXIC```|```3```|The by Fractions Count on Lexical Key Order Sequence |
|```FC_VOLUME```|```4```|The by Fractions Count on Volumetric Key Order Sequence |

### Enumeration: ColumnType
Column Value Types 

|Name|Value|Description|
|---|---|---|
|```UNKNOWN```|```0```|Unknown/Unrecognized Type |
|```PLAIN```|```1```|A Plain Column Value |
|```COUNTER_I64```|```2```|A Counter Column Value with integrity of signed-64bit |
|```COUNTER_I32```|```3```|A Counter Column Value with integrity of signed-32bit |
|```COUNTER_I16```|```4```|A Counter Column Value with integrity of signed-16bit |
|```COUNTER_I8```|```5```|A Counter Column Value with integrity of signed-8bit |
|```SERIAL```|```6```|A Serial Column Value |
|```CELL_DEFINED```|```15```|Not used - experimental |

### Enumeration: EncodingType
Data Encoding Types 

|Name|Value|Description|
|---|---|---|
|```DEFAULT```|```0```|Encoding by Ranger DEFAULT configurations |
|```PLAIN```|```1```|No Encoding |
|```ZLIB```|```2```|Encode with zlib |
|```SNAPPY```|```3```|Encode with snappy |
|```ZSTD```|```4```|Encode with zstandard |
|```UNKNOWN```|```255```|Unrecognized Type |

### Enumeration: SchemaFunc
Manage Columns schema function Flags 

|Name|Value|Description|
|---|---|---|
|```CREATE```|```3```|Create Column Function |
|```REMOVE```|```5```|Delete Column Function |
|```MODIFY```|```7```|Modify Column Function |

### Enumeration: Comp
The available logical Comparators, plus extended logic options applied with 'v' for VOLUME 

|Name|Value|Description|
|---|---|---|
|```NONE```|```0```|[         ]  :   none               (no comparison applied) |
|```PF```|```1```|[  =^     ]  :   -pf [prefix]       (starts-with) |
|```GT```|```2```|[ &gt;    ]  :   -gt                (greater-than) |
|```GE```|```3```|[ &gt;=   ]  :   -ge                (greater-equal) |
|```EQ```|```4```|[  =      ]  :   -eq                (equal) |
|```LE```|```5```|[ &lt;=   ]  :   -le                (lower-equal) |
|```LT```|```6```|[ &lt;    ]  :   -lt                (lower-than) |
|```NE```|```7```|[  !=     ]  :   -ne                (not-equal) |
|```RE```|```8```|[  re     ]  :   -re [r,regexp]     (regular-expression) |
|```VGT```|```9```|[ v&gt;   ]  :   -vgt               (vol greater-than) |
|```VGE```|```10```|[ v&gt;=  ]  :   -vge               (vol greater-equal) |
|```VLE```|```11```|[ v&lt;=  ]  :   -vle               (vol lower-equal) |
|```VLT```|```12```|[ v&lt;   ]  :   -vlt               (vol lower-than) |
|```SBS```|```13```|[ %&gt;   ]  :   -subset [sbs]      (subset) |
|```SPS```|```14```|[ &lt;%   ]  :   -supset [sps]      (superset) |
|```POSBS```|```15```|[ ~&gt;   ]  :   -posubset [posbs]  (eq/part ordered subset) |
|```POSPS```|```16```|[ &lt;~   ]  :   -posupset [posps]  (eq/part ordered superset) |
|```FOSBS```|```17```|[ -&gt;   ]  :   -fosubset [fosbs]  (eq/full ordered subset) |
|```FOSPS```|```18```|[ &lt;-   ]  :   -fosupset [fosps]  (eq/full ordered superset) |
|```FIP```|```19```|[ :&lt;   ]  :   -fip  (fraction include prior) |
|```FI```|```20```|[ :       ]  :   -fi   (fraction include) |

### Enumeration: SpecFlagsOpt
The Scan options Flags Specifications for the SpecFlags 'options' bit 

|Name|Value|Description|
|---|---|---|
|```NONE```|```0```|No Flag Applied |
|```LIMIT_BY_KEYS```|```1```|Cells Limit by Keys |
|```OFFSET_BY_KEYS```|```4```|Cells Offset by Keys |
|```ONLY_KEYS```|```8```|Select Cells Only Keys without Value data |
|```ONLY_DELETES```|```10```|Select Cells Only with DELETE(cell-flag) |

### Enumeration: SpecIntervalOptions
The Scan Interval Specs Options for the SpecInterval and SpecIntervalSerial 'options' bit 

|Name|Value|Description|
|---|---|---|
|```UPDATING```|```4```|Update Bit Option |
|```DELETING```|```8```|Delete Bit Option |

### Enumeration: UpdateOP


|Name|Value|Description|
|---|---|---|
|```REPLACE```|```0```|The OP supported by column-types: PLAIN, SERIAL, COUNTER. Replaces with the update value (_default as well if other OP not supported by the col-type_) |
|```APPEND```|```1```|The OP supported by column-types: PLAIN, SERIAL. Appends the update value to the cell's current |
|```PREPEND```|```2```|The OP supported by column-types: PLAIN, SERIAL. Prepends the update value to the cell's current |
|```INSERT```|```3```|The OP supported by column-type PLAIN. Inserts the update value at position in current value (appends if pos above value) |
|```OVERWRITE```|```4```|The OP supported by column-type PLAIN. Overwrites the current value at position with new value (appends if pos above value) |
|```SERIAL```|```5```|The OP supported by column-type SERIAL. update is done by the inner serial-fields defintions |

### Enumeration: Flag
The Cell Flag 

|Name|Value|Description|
|---|---|---|
|```NONE```|```0```|Unknown/Undefined |
|```INSERT```|```1```|The Cell is an insert |
|```DELETE_LE```|```2```|The Cell is a delete versions lower-equal |
|```DELETE_EQ```|```3```|The Cell is a  delete version equal |

### Enumeration: FU_MATH_OP
MATH Operations for Serial Field Update of types INT64 and DOUBLE 

|Name|Value|Description|
|---|---|---|
|```EQUAL```|```0```|set field value to the new value |
|```PLUS```|```1```|plus new value to field's value (negative number allowed) |
|```MULTIPLY```|```2```|multiply current value by update value |
|```DIVIDE```|```3```|divide current value by the new value (ignored at zero) |

### Enumeration: FU_LIST_OP
LIST Operations for Serial Field Update of array/list/bytes with LIST-op in the inner SERIAL fields 

|Name|Value|Description|
|---|---|---|
|```REPLACE```|```0```|Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Replaces with the update value |
|```APPEND```|```1```|Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Appends the update value to a field value |
|```PREPEND```|```2```|Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Prepends the update value to a field value |
|```INSERT```|```3```|Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Insert the update value at position in a field value (appends if pos above value) |
|```OVERWRITE```|```4```|Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Overwrites a field value at position with new value (appends if pos above value) |
|```ERASE```|```5```|Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Erases the position in a field value |
|```BY_UNIQUE```|```6```|Supported by field-types: LIST_BYTES, LIST_INT64. The field value items have CTRL_VALUE_SET/DEL OP |
|```BY_COND```|```7```|Supported by field-types: LIST_BYTES, LIST_INT64. The field value items have CTRL_VALUE_SET/DEL OP and Comparator |
|```BY_INDEX```|```8```|Supported by field-types: LIST_BYTES, LIST_INT64. The field value is with Postion and OP in items |

### Enumeration: CellsResult
The Cells Results types for using with CellsGroup requests 

|Name|Value|Description|
|---|---|---|
|```IN_LIST```|```0```|Correspond to result on Cells (Cells in list) |
|```ON_COLUMN```|```1```|Correspond to result on CCells (Columns Cells) |
|```ON_KEY```|```2```|Correspond to result on KCells (Keys Cells) |
|```ON_FRACTION```|```3```|Correspond to result on FCells (Fraction Cells) |

***
## Type declarations

### Typedef: Schemas
A list-container of Schemas 

_Base type_: **list&lt;[```Schema```](#struct-schema)&gt;**


### Typedef: Key
A Cell Key defined as binary(bytes) items in a list-container 

_Base type_: **list&lt;```binary```&gt;**


### Typedef: SpecKey
The Key Specifications defined as SpecFraction items in a list-container 

_Base type_: **list&lt;[```SpecFraction```](#struct-specfraction)&gt;**


### Typedef: SpecKeyIntervals
The Key Intervals Specifications defined as SpecKeyInterval items in a list-container 

_Base type_: **list&lt;[```SpecKeyInterval```](#struct-speckeyinterval)&gt;**


### Typedef: SpecValues
The Cell Value Specifications defined as SpecValue items in a list-container 

_Base type_: **list&lt;[```SpecValue```](#struct-specvalue)&gt;**


### Typedef: SpecValueSerialFields
The Serial Cell Value Specifications defined as SpecValueSerialField items in a list-container 

_Base type_: **list&lt;[```SpecValueSerialField```](#struct-specvalueserialfield)&gt;**


### Typedef: SpecValuesSerial
The Cell Value Specifications defined as SpecValueSerial items in a list-container 

_Base type_: **list&lt;[```SpecValueSerial```](#struct-specvalueserial)&gt;**


### Typedef: UCells
The Cells for Update defined as UCell items in a list-container 

_Base type_: **list&lt;[```UCell```](#struct-ucell)&gt;**


### Typedef: UCCells
The Cells for Update for a Column Id defined as UCells items in a map-container by CID 

_Base type_: **map&lt;```i64```, [```UCells```](#typedef-ucells)&gt;**


### Typedef: CellValuesSerial
The Serial Cell Value Fields defined as CellValueSerial items in a list-container 

_Base type_: **list&lt;[```CellValueSerial```](#struct-cellvalueserial)&gt;**


### Typedef: CellValuesSerialOp
The Serial Cell Value Fields defined as CellValueSerialOp items in a list-container 

_Base type_: **list&lt;[```CellValueSerialOp```](#struct-cellvalueserialop)&gt;**


### Typedef: UCellsSerial
The Cells for Update defined as UCellSerial items in a list-container 

_Base type_: **list&lt;[```UCellSerial```](#struct-ucellserial)&gt;**


### Typedef: UCCellsSerial
The Cells for Update for a Column Id defined as UCellsSerial items in a map-container by CID 

_Base type_: **map&lt;```i64```, [```UCellsSerial```](#typedef-ucellsserial)&gt;**


### Typedef: CCells
The Columns Cells for results on Columns of scan, defined as ColCells items in a map-container by Column Name 

_Base type_: **map&lt;```string```, [```ColCells```](#struct-colcells)&gt;**


### Typedef: KCells
The Keys Cells for results on Key of scan, defined as kCells items in a list-container 

_Base type_: **list&lt;[```kCells```](#struct-kcells)&gt;**


### Typedef: CompactResults
The Compact Results, defined as CompactResult items in a list-container 

_Base type_: **list&lt;[```CompactResult```](#struct-compactresult)&gt;**


***
## Data structures

### Exception: Exception
The SWC::Thrift::Exception a base for any Exceptions both for the Thrift-Protocol and SWC-DB Errors. 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|code|```i32```|The corresponding Thrift-Procotol or SWC-DB Error Code |default||
|2|message|```string```|The message describing the error code |default||

### Struct: Schema
The Schema Definition 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cid|```i64```|Column ID |optional||
|2|col_name|```string```|Column Name |optional||
|3|col_tags|list&lt;```string```&gt;|Column Tags |default||
|4|col_seq|[```KeySeq```](#enumeration-keyseq)|Column Key Sequence |optional||
|5|col_type|[```ColumnType```](#enumeration-columntype)|Column Type |optional||
|6|cell_versions|```i32```|Cell Versions |optional||
|7|cell_ttl|```i32```|Cell Time to Live |optional||
|8|blk_encoding|[```EncodingType```](#enumeration-encodingtype)|Block Encoding |optional||
|9|blk_size|```i32```|Block Size in Bytes |optional||
|10|blk_cells|```i32```|Number of Cells in Block |optional||
|11|cs_replication|```i8```|CellStore file Replication |optional||
|12|cs_size|```i32```|CellStore Size in Bytes |optional||
|13|cs_max|```i8```|Max CellStores in a Range |optional||
|14|log_rollout_ratio|```i8```|Write Fragment File on ratio reached |optional||
|15|log_compact_cointervaling|```i8```|Min. Cointervaling Fragments for Compaction |optional||
|16|log_fragment_preload|```i8```|Number of Fragment to Preload |optional||
|17|compact_percent|```i8```|Compact at percent reach |optional||
|18|revision|```i64```|Schema's revision/id |optional||

### Struct: SchemaPattern
The Schema matching Pattern 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|value|```string```|The patern value to match against |default||

### Struct: SchemaTagsPatterns
The Schema Tags patterns for the SchemaPatterns 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply, unsupported PF, RE and Vol. kind |default||
|2|values|list&lt;[```SchemaPattern```](#struct-schemapattern)&gt;|The tags patterns to match against schema's column tags |default||

### Struct: SchemaPatterns
The Schema Patterns for the SpecSchemas 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|names|list&lt;[```SchemaPattern```](#struct-schemapattern)&gt;|The Schema patterns for selecting by Column Name |default||
|2|tags|[```SchemaTagsPatterns```](#struct-schematagspatterns)|The Schema patterns for selecting by Column Tags |default||

### Struct: SpecSchemas
The Specs for Schemas for using with list_columns or compact_columns 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cids|list&lt;```i64```&gt;|The Column IDs |default||
|2|names|list&lt;```string```&gt;|The Column Names |default||
|3|patterns|[```SchemaPatterns```](#struct-schemapatterns)|The Schema's selector patterns |default||

### Struct: SpecFlags
The Scan Specifications Flags 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|limit|```i64```|Limit to this number of cells |optional||
|2|offset|```i64```|Scan from this number of cells Offset on matching Cell-Interval |optional||
|3|max_versions|```i32```|Select only this number of Versions of a given Cell-Key |optional||
|4|max_buffer|```i32```|return results with reach of this Buffer size in bytes |optional||
|5|options|```i8```|The options bit by SpecFlagsOpt |optional||

### Struct: SpecFraction
The Fraction Specifications 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|f|```binary```|The binary(bytes) to match against a fraction of a Cell-Key |default||

### Struct: SpecTimestamp
The Timestamp Specifications 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|ts|```i64```|The timestamp in nanoseconds to match against the Cell timestamp/version (not the revision) |default||

### Struct: SpecKeyInterval
The Key Interval Specifications 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|start|[```SpecKey```](#typedef-speckey)|The Key Start Spec, the start of cells-interval key match |default||
|2|finish|[```SpecKey```](#typedef-speckey)|The Key Finish Spec, the finish of cells-interval key match |default||

### Struct: SpecValue
The Value Specifications, option to use with Extended Logical Comparators 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|v|```binary```|The binary(bytes) to match against the Cell value |default||

### Struct: SpecUpdateOP


| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|op|[```UpdateOP```](#enumeration-updateop)|The Operation of update |default||
|2|pos|```i32```|The position/index of INSERT and OVERWRITE update operations |optional||

### Struct: SpecIntervalUpdate
The Value specs for an Updating Interval of 'updating' in SpecInterval 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|v|```binary```|The value for the updated cell |default||
|2|ts|```i64```|The timestamp for the updated cell NULL: MIN_INT64+1, AUTO:MIN_INT64+2 (or not-set) |optional||
|3|encoder|[```EncodingType```](#enumeration-encodingtype)|Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD |optional||
|4|update_op|[```SpecUpdateOP```](#struct-specupdateop)|Optionally the operaton of value update |optional||

### Struct: SpecIntervalUpdateSerial
The Value specs for an Updating Interval of 'updating' in SpecIntervalSerial 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|ts|```i64```|The timestamp for the updated cell NULL: MIN_INT64-1, AUTO:MIN_INT64-1 |default||
|2|v|[```CellValuesSerial```](#typedef-cellvaluesserial)|The values of serial-fields for the updated cell |default||
|3|v_op|[```CellValuesSerialOp```](#typedef-cellvaluesserialop)|The values of serial-fields for the the SERIAL operation update |default||
|4|encoder|[```EncodingType```](#enumeration-encodingtype)|Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD |optional||
|5|update_op|[```SpecUpdateOP```](#struct-specupdateop)|Optionally the operaton of value update |optional||

### Struct: SpecInterval
The Cells Interval Specifications with interval-scope Flags 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|range_begin|[```Key```](#typedef-key)|Begin of Ranges evaluation with this Key inclusive |default||
|2|range_end|[```Key```](#typedef-key)|End of Ranges evaluation with this Key inclusive |default||
|3|offset_key|[```Key```](#typedef-key)|Offset Cell Key of a Scan, select cells from this key inclusive |default||
|4|offset_rev|```i64```|Offset Cell Timestamp of a Scan, select cells after this timestamp |optional||
|5|key_intervals|[```SpecKeyIntervals```](#typedef-speckeyintervals)|The Key Intervals |default||
|6|values|[```SpecValues```](#typedef-specvalues)|The Cell Value Specifications, cell-value match |default||
|7|ts_start|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Start Spec, the start of cells-interval timestamp match |optional||
|8|ts_finish|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Finish Spec, the finish of cells-interval timestamp match |optional||
|9|flags|[```SpecFlags```](#struct-specflags)|The Interval Flags Specification |optional||
|10|options|[```SpecIntervalOptions```](#enumeration-specintervaloptions)|The Interval Options Specification |optional||
|11|updating|[```SpecIntervalUpdate```](#struct-specintervalupdate)|The Value spec of an Updating Interval |optional||

### Struct: SpecColumn
The Column Specifications, the Cells-Intervals(SpecInterval/s) specification for a column 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cid|```i64```|The Column ID |default||
|2|intervals|list&lt;[```SpecInterval```](#struct-specinterval)&gt;|The Cells Interval in a list-container |default||

### Struct: SpecValueSerial_INT64
The Specifications of INT64 Serial Value Field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|v|```i64```|The int64 to match against the value field |default||

### Struct: SpecValueSerial_DOUBLE
The Specifications of DOUBLE Serial Value Field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|v|```double```|The double to match against the value field |default||

### Struct: SpecValueSerial_BYTES
The Specifications of BYTES Serial Value Field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|v|```binary```|The binary(bytes) to match against the value field |default||

### Struct: SpecValueSerial_KEY
The Specifications of KEY Serial Value Field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|seq|[```KeySeq```](#enumeration-keyseq)|The Key Sequence to use |default||
|2|v|[```SpecKey```](#typedef-speckey)|The Specification of the Key to match against the value field |default||

### Struct: SpecValueSerial_LI
The Specifications of LIST_INT64(LI) Serial Value Field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|v|list&lt;[```SpecValueSerial_INT64```](#struct-specvalueserial_int64)&gt;|The List of Int64 to match against the value field |default||

### Struct: SpecValueSerial_LB
The Specifications of LIST_BYTES(LB) Serial Value Field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|v|list&lt;[```SpecValueSerial_BYTES```](#struct-specvalueserial_bytes)&gt;|The List of Bytes to match against the value field |default||

### Struct: SpecValueSerialField


| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|field_id|```i32```|The Field Id of the Value Field |default||
|2|spec_int64|[```SpecValueSerial_INT64```](#struct-specvalueserial_int64)|The specifications of Int64 for the field |optional||
|3|spec_double|[```SpecValueSerial_DOUBLE```](#struct-specvalueserial_double)|The specifications of Double for the field |optional||
|4|spec_bytes|[```SpecValueSerial_BYTES```](#struct-specvalueserial_bytes)|The specifications of Bytes for the field |default||
|5|spec_key|[```SpecValueSerial_KEY```](#struct-specvalueserial_key)|The specifications of Cell-Key for the field |default||
|6|spec_li|[```SpecValueSerial_LI```](#struct-specvalueserial_li)|The specifications of List Int64 for the field |default||
|7|spec_lb|[```SpecValueSerial_LB```](#struct-specvalueserial_lb)|The specifications of List Bytes for the field |default||

### Struct: SpecValueSerial
The Serial Value Specifications 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|fields|[```SpecValueSerialFields```](#typedef-specvalueserialfields)|The Serial Value Specifications to match against the SERIAL Cell value fields |default||

### Struct: SpecIntervalSerial
The Serial Value Cells Interval Specifications with interval-scope Flags 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|range_begin|[```Key```](#typedef-key)|Begin of Ranges evaluation with this Key inclusive |default||
|2|range_end|[```Key```](#typedef-key)|End of Ranges evaluation with this Key inclusive |default||
|3|offset_key|[```Key```](#typedef-key)|Offset Cell Key of a Scan, select cells from this key inclusive |default||
|4|offset_rev|```i64```|Offset Cell Timestamp of a Scan, select cells after this timestamp |optional||
|5|key_intervals|[```SpecKeyIntervals```](#typedef-speckeyintervals)|The Key Intervals |default||
|6|values|[```SpecValuesSerial```](#typedef-specvaluesserial)|The Serial Cell Value Specifications, cell-value fields match |default||
|7|ts_start|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Start Spec, the start of cells-interval timestamp match |optional||
|8|ts_finish|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Finish Spec, the finish of cells-interval timestamp match |optional||
|9|flags|[```SpecFlags```](#struct-specflags)|The Interval Flags Specification |optional||
|10|options|[```SpecIntervalOptions```](#enumeration-specintervaloptions)|The Interval Options Specification |optional||
|11|updating|[```SpecIntervalUpdateSerial```](#struct-specintervalupdateserial)|The Serial-Value spec of an Updating Interval |optional||

### Struct: SpecColumnSerial
The Column Specifications, the Cells-Intervals(SpecInterval/s) specification for a SERIAL Type Column 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cid|```i64```|The Column ID |default||
|2|intervals|list&lt;[```SpecIntervalSerial```](#struct-specintervalserial)&gt;|The Serial Cells Interval in a list-container |default||

### Struct: SpecScan
The Scan Specifications, the Columns-Intervals(SpecColumn/s) with global-scope Flags 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|columns|list&lt;[```SpecColumn```](#struct-speccolumn)&gt;|The Column Intervals(SpecColumn) in a list-container |default||
|2|columns_serial|list&lt;[```SpecColumnSerial```](#struct-speccolumnserial)&gt;|The Serial Column Intervals(SpecColumnSerial) in a list-container |default||
|3|flags|[```SpecFlags```](#struct-specflags)|The Global Flags Specification |optional||

### Struct: UCell
The Cell data for using with Update 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|f|[```Flag```](#enumeration-flag)|The Cell Flag |default||
|2|k|[```Key```](#typedef-key)|The Cell Key |default||
|3|ts|```i64```|The Cell Timestamp in nanoseconds |optional||
|4|ts_desc|```bool```|The Cell Version is in timestamp descending |optional||
|5|v|```binary```|The Cell Value |default||
|6|encoder|[```EncodingType```](#enumeration-encodingtype)|Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD |optional||

### Struct: CellValueSerial
The Serial Value Cell field 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|field_id|```i32```|The Field ID, a single ID can have any/all the field types |default||
|2|v_int64|```i64```|The INT64 type field |optional||
|3|v_double|```double```|The DOUBLE type field |optional||
|4|v_bytes|```binary```|The BYTES type field |default||
|5|v_key|[```Key```](#typedef-key)|The Cell KEY type field |default||
|6|v_li|list&lt;```i64```&gt;|The LIST INT64 type field |default||
|7|v_lb|list&lt;```binary```&gt;|The LIST BYTES type field |default||

### Struct: FU_INT64
Serial INT64 Field Update 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|ctrl|```i8```||default|``````0``````|
|2|op|[```FU_MATH_OP```](#enumeration-fu_math_op)||default|```[```FU_MATH_OP.EQUAL```](#constant-fu_math_opequal)```|
|3|pos|```i32```||optional||
|4|comp|[```Comp```](#enumeration-comp)||optional||
|5|v|```i64```||default||

### Struct: FU_DOUBLE
Serial DOUBLE Field Update 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|ctrl|```i8```||default|``````0``````|
|2|op|[```FU_MATH_OP```](#enumeration-fu_math_op)||default|```[```FU_MATH_OP.EQUAL```](#constant-fu_math_opequal)```|
|3|pos|```i32```||optional||
|4|comp|[```Comp```](#enumeration-comp)||optional||
|5|v|```double```||default||

### Struct: FU_BYTES
Serial BYTES Field Update 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|ctrl|```i8```||default|``````0``````|
|2|op|[```FU_LIST_OP```](#enumeration-fu_list_op)||default|```[```FU_LIST_OP.REPLACE```](#constant-fu_list_opreplace)```|
|3|pos|```i32```||optional||
|4|comp|[```Comp```](#enumeration-comp)||optional||
|5|v|```binary```||default||

### Struct: FU_LI
Serial LIST_INT64 Field Update 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|ctrl|```i8```||default|``````0``````|
|2|op|[```FU_LIST_OP```](#enumeration-fu_list_op)||default|```[```FU_LIST_OP.REPLACE```](#constant-fu_list_opreplace)```|
|3|pos|```i32```||optional||
|4|v|list&lt;[```FU_INT64```](#struct-fu_int64)&gt;||default||

### Struct: FU_LB
Serial LIST_BYTES Field Update 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|ctrl|```i8```||default|``````0``````|
|2|op|[```FU_LIST_OP```](#enumeration-fu_list_op)||default|```[```FU_LIST_OP.REPLACE```](#constant-fu_list_opreplace)```|
|3|pos|```i32```||optional||
|4|v|list&lt;[```FU_BYTES```](#struct-fu_bytes)&gt;||default||

### Struct: CellValueSerialOp
The Serial Values Cell field with Update Operation 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|field_id|```i32```|The Field ID, a single ID can have any/all the field types |default||
|2|v_int64|[```FU_INT64```](#struct-fu_int64)|The INT64 type update-field |optional||
|3|v_double|[```FU_DOUBLE```](#struct-fu_double)|The DOUBLE type update-field |optional||
|4|v_bytes|[```FU_BYTES```](#struct-fu_bytes)|The BYTES type update-field |optional||
|5|v_key|[```Key```](#typedef-key)|The Cell KEY type update-field |default||
|6|v_li|[```FU_LI```](#struct-fu_li)|The LIST INT64 type update-field |optional||
|7|v_lb|[```FU_LB```](#struct-fu_lb)|The LIST BYTES type update-field |optional||

### Struct: UCellSerial
The Cell data for using with Update of SERIAL Column Type 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|f|[```Flag```](#enumeration-flag)|The Cell Flag |default||
|2|k|[```Key```](#typedef-key)|The Cell Key |default||
|3|ts|```i64```|The Cell Timestamp in nanoseconds |optional||
|4|ts_desc|```bool```|The Cell Version is in timestamp descending |optional||
|5|v|[```CellValuesSerial```](#typedef-cellvaluesserial)|The Serial Cell Value fields |default||
|6|encoder|[```EncodingType```](#enumeration-encodingtype)|Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD |optional||

### Struct: Cell
The Cell for results list of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|c|```string```|The Column Name |default||
|2|k|[```Key```](#typedef-key)|The Cell Key |default||
|3|ts|```i64```|The Cell Timestamp |default||
|4|v|```binary```|The Cell Value |default||

### Struct: CellSerial
The Serial Cell for results list of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|c|```string```|The Column Name |default||
|2|k|[```Key```](#typedef-key)|The Cell Key |default||
|3|ts|```i64```|The Cell Timestamp |default||
|4|v|[```CellValuesSerial```](#typedef-cellvaluesserial)|The Cell Serial Value |default||

### Struct: Cells
The Cells for results list of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cells|list&lt;[```Cell```](#struct-cell)&gt;|The Cells, defined as Cell items in a list-container |default||
|2|serial_cells|list&lt;[```CellSerial```](#struct-cellserial)&gt;|The Serial Cells, defined as CellSerial items in a list-container |default||

### Struct: CCell
The Column Cell for results on Columns of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|k|[```Key```](#typedef-key)|The Cell Key |default||
|2|ts|```i64```|The Cell Timestamp |default||
|3|v|```binary```|The Cell Value |default||

### Struct: CCellSerial
The Column Serial Cell for results on Columns of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|k|[```Key```](#typedef-key)|The Cell Key |default||
|2|ts|```i64```|The Cell Timestamp |default||
|3|v|[```CellValuesSerial```](#typedef-cellvaluesserial)|The Cell Serial Value |default||

### Struct: ColCells
The Column Cells for results on Columns of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cells|list&lt;[```CCell```](#struct-ccell)&gt;|The Cells, defined as CCell items in a list-container |default||
|2|serial_cells|list&lt;[```CCellSerial```](#struct-ccellserial)&gt;|The Serial Cells, defined as CCellSerial items in a list-container |default||

### Struct: KCell
The Key Cell for results on Key of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|c|```string```|The Column Name |default||
|2|ts|```i64```|The Cell Timestamp |default||
|3|v|```binary```|The Cell Value |default||

### Struct: KCellSerial
The Key Serial Cell for results on Key of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|c|```string```|The Column Name |default||
|2|ts|```i64```|The Cell Timestamp |default||
|3|v|[```CellValuesSerial```](#typedef-cellvaluesserial)|The Cell Serial Value |default||

### Struct: kCells
The Key Cells for results on Key of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|k|[```Key```](#typedef-key)|The Cell Key |default||
|2|cells|list&lt;[```KCell```](#struct-kcell)&gt;|The Key's Cells, defined as KCell items in a list-container |default||
|3|serial_cells|list&lt;[```KCellSerial```](#struct-kcellserial)&gt;|The Key's Serial Cells, defined as KCellSerial items in a list-container |default||

### Struct: FCell
The Fraction Cell for results on Fraction of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|c|```string```|The Column Name |default||
|2|ts|```i64```|The Cell Timestamp |default||
|3|v|```binary```|The Cell Value |default||

### Struct: FCellSerial
The Fraction Serial Cell for results on Fraction of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|c|```string```|The Column Name |default||
|2|ts|```i64```|The Cell Timestamp |default||
|3|v|[```CellValuesSerial```](#typedef-cellvaluesserial)|The Cell Serial Value |default||

### Struct: FCells
The Fraction Cells for results on Fraction of scan 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|f|map&lt;```binary```, [```FCells```](#typedef-fcells)&gt;|The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes |default||
|2|cells|list&lt;[```FCell```](#struct-fcell)&gt;|The current Fraction's Cells, defined as FCell items in a list-container |default||
|3|serial_cells|list&lt;[```FCellSerial```](#struct-fcellserial)&gt;|The current Fraction's Serial Cells, defined as FCellSerial items in a list-container |default||

### Struct: CellsGroup
A Grouped Cells result for results of scan, determined by the request's CellsResult enum 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cells|[```Cells```](#struct-cells)|The Cells in a list, defined as Cell items in a list-container |default||
|2|ccells|[```CCells```](#typedef-ccells)|The Columns Cells in a map-container, defined as ColCells items by Column Name |default||
|3|kcells|[```KCells```](#typedef-kcells)|The Keys Cells in a list, defined as kCells items in a list-container |default||
|4|fcells|[```FCells```](#struct-fcells)|The Fraction Cells in struct FCells |default||

### Struct: CompactResult
The Compact Result 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cid|```i64```|Column ID |default||
|2|err|```i32```|Error |default||

### Struct: Result
The Result of 'exec_sql' 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|schemas|[```Schemas```](#typedef-schemas)|Set with result for 'list columns' query |default||
|2|cells|[```Cells```](#struct-cells)|Set with result for 'select' query |default||
|3|compact|[```CompactResults```](#typedef-compactresults)|Set with result for 'compact columns' query |default||

***
## Services

### Service: Service
The SWC-DB Thrift Service 
#### Function: Service.sql_mng_column
The direct SQL method to Manage Column. 

```void```
 _sql_mng_column_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_list_columns
The direct SQL method to List Columns 

[```Schemas```](#typedef-schemas)
 _sql_list_columns_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_compact_columns
The direct SQL method to Compact Columns 

[```CompactResults```](#typedef-compactresults)
 _sql_compact_columns_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_select
The direct SQL method to select cells with result in Cells List. 

[```Cells```](#struct-cells)
 _sql_select_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_select_rslt_on_column
The direct SQL method to select cells with result in Columns Cells map. 

[```CCells```](#typedef-ccells)
 _sql_select_rslt_on_column_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_select_rslt_on_key
The direct SQL method to select cells with result in Key Cells list. 

[```KCells```](#typedef-kcells)
 _sql_select_rslt_on_key_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_select_rslt_on_fraction
The direct SQL method to select cells with result in Fractons Cells. 

[```FCells```](#struct-fcells)
 _sql_select_rslt_on_fraction_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.sql_query
The SQL method to select cells with result set by the request's type of CellsResult. 

[```CellsGroup```](#struct-cellsgroup)
 _sql_query_(```string``` sql,
[```CellsResult```](#enumeration-cellsresult) rslt)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 
2. rslt - The Type of Cells Result for the response 


#### Function: Service.sql_update
The direct SQL method to update cells optionally to work with updater-id. 

```void```
 _sql_update_(```string``` sql,
```i64``` updater_id = ```0```)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 
2. updater_id - The Updater ID to work with 


#### Function: Service.exec_sql
The SQL method to execute any query. 

[```Result```](#struct-result)
 _exec_sql_(```string``` sql)
> throws [```Exception```](#exception-exception)

* parameters:
1. sql - The SQL string to Execute 


#### Function: Service.updater_create
The method to Create an Updater ID with buffering size in bytes. 

```i64```
 _updater_create_(```i32``` buffer_size)
> throws [```Exception```](#exception-exception)

* parameters:
1. buffer_size - The buffer size of the Updater 


#### Function: Service.updater_close
The method to Close an Updater ID. 

```void```
 _updater_close_(```i64``` id)
> throws [```Exception```](#exception-exception)

* parameters:
1. id - The Updater ID to close 


#### Function: Service.update
The direct method to update cells with cell in Update-Columns-Cells, optionally to work with updater-id. 

```void```
 _update_([```UCCells```](#typedef-uccells) cells,
```i64``` updater_id = ```0```)
> throws [```Exception```](#exception-exception)

* parameters:
1. cells - The Cells to update 
2. updater_id - The Updater ID to use for write 


#### Function: Service.update_serial
The direct method to update cells with cell in Update-Columns-Cells-Serial, optionally to work with updater-id. 

```void```
 _update_serial_([```UCCellsSerial```](#typedef-uccellsserial) cells,
```i64``` updater_id = ```0```)
> throws [```Exception```](#exception-exception)

* parameters:
1. cells - The Serial Cells to update 
2. updater_id - The Updater ID to use for write 


#### Function: Service.update_by_types
The method is to update cells by several Column-Types, optionally to work with updater-id. 

```void```
 _update_by_types_([```UCCells```](#typedef-uccells) plain,
[```UCCellsSerial```](#typedef-uccellsserial) serial,
```i64``` updater_id = ```0```)
> throws [```Exception```](#exception-exception)

* parameters:
1. plain - The PLAIN Cells to update 
2. serial - The SERIAL Cells to update 
3. updater_id - The Updater ID to use for write 


#### Function: Service.mng_column
The direct method to Manage Column 

```void```
 _mng_column_([```SchemaFunc```](#enumeration-schemafunc) func,
[```Schema```](#struct-schema) schema)
> throws [```Exception```](#exception-exception)

* parameters:
1. func - The Action Function to use 
2. schema - The Schema for the Action 


#### Function: Service.list_columns
The direct method to List Columns 

[```Schemas```](#typedef-schemas)
 _list_columns_([```SpecSchemas```](#struct-specschemas) spec)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Schemas Specifications to match Schema for response 


#### Function: Service.compact_columns
The direct method to Compact Columns 

[```CompactResults```](#typedef-compactresults)
 _compact_columns_([```SpecSchemas```](#struct-specschemas) spec)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Schemas Specifications to match columns to Compact 


#### Function: Service.scan
The direct method to select cells with result in Cells List. 

[```Cells```](#struct-cells)
 _scan_([```SpecScan```](#struct-specscan) spec)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Scan Specifications for the scan 


#### Function: Service.scan_rslt_on_column
The direct method to select cells with result in Columns Cells map. 

[```CCells```](#typedef-ccells)
 _scan_rslt_on_column_([```SpecScan```](#struct-specscan) spec)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Scan Specifications for the scan 


#### Function: Service.scan_rslt_on_key
The direct method to select cells with result in Key Cells list. 

[```KCells```](#typedef-kcells)
 _scan_rslt_on_key_([```SpecScan```](#struct-specscan) spec)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Scan Specifications for the scan 


#### Function: Service.scan_rslt_on_fraction
The direct method to select cells with result in Fractons Cells. 

[```FCells```](#struct-fcells)
 _scan_rslt_on_fraction_([```SpecScan```](#struct-specscan) spec)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Scan Specifications for the scan 


#### Function: Service.scan_rslt_on
The method to select cells with result set by the request's type of CellsResult. 

[```CellsGroup```](#struct-cellsgroup)
 _scan_rslt_on_([```SpecScan```](#struct-specscan) spec,
[```CellsResult```](#enumeration-cellsresult) rslt)
> throws [```Exception```](#exception-exception)

* parameters:
1. spec - The Scan Specifications for the scan 
2. rslt - The Type of Cells Result for the response 

* exceptions:
  * Exception - The Base Exception 





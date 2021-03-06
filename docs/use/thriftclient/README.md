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
  * [C-Glib](c_glib/)

 _**Additional only generated SWC-DB Thrift Client Service**_:

  * [Netstd](netstd/)
  * [Rust](rust/)


> Languages Support can be extended upto [supported languages by Apache-Thrift ](https://github.com/apache/thrift/blob/master/LANGUAGES.md)



*** 



## _The SWC-DB Thrift Modules_




| Module | Services & Functions | Data types | Constants |
| --- | --- | --- | --- |
|Service|[Service](#service-service)|[KeySeq](#enumeration-keyseq)||
||	[ &bull; sql_mng_column](#function-servicesql_mng_column)|[ColumnType](#enumeration-columntype)||
||	[ &bull; sql_list_columns](#function-servicesql_list_columns)|[EncodingType](#enumeration-encodingtype)||
||	[ &bull; sql_compact_columns](#function-servicesql_compact_columns)|[SchemaFunc](#enumeration-schemafunc)||
||	[ &bull; sql_select](#function-servicesql_select)|[Comp](#enumeration-comp)||
||	[ &bull; sql_select_rslt_on_column](#function-servicesql_select_rslt_on_column)|[SpecFlagsOpt](#enumeration-specflagsopt)||
||	[ &bull; sql_select_rslt_on_key](#function-servicesql_select_rslt_on_key)|[Flag](#enumeration-flag)||
||	[ &bull; sql_select_rslt_on_fraction](#function-servicesql_select_rslt_on_fraction)|[CellsResult](#enumeration-cellsresult)||
||	[ &bull; sql_query](#function-servicesql_query)|[Schemas](#typedef-schemas)||
||	[ &bull; sql_update](#function-servicesql_update)|[Key](#typedef-key)||
||	[ &bull; exec_sql](#function-serviceexec_sql)|[SpecKey](#typedef-speckey)||
||	[ &bull; updater_create](#function-serviceupdater_create)|[SpecKeyIntervals](#typedef-speckeyintervals)||
||	[ &bull; updater_close](#function-serviceupdater_close)|[SpecValues](#typedef-specvalues)||
||	[ &bull; update](#function-serviceupdate)|[SpecValueSerialFields](#typedef-specvalueserialfields)||
||	[ &bull; update_serial](#function-serviceupdate_serial)|[SpecValuesSerial](#typedef-specvaluesserial)||
||	[ &bull; mng_column](#function-servicemng_column)|[UCells](#typedef-ucells)||
||	[ &bull; list_columns](#function-servicelist_columns)|[UCCells](#typedef-uccells)||
||	[ &bull; compact_columns](#function-servicecompact_columns)|[CellValuesSerial](#typedef-cellvaluesserial)||
||	[ &bull; scan](#function-servicescan)|[UCellsSerial](#typedef-ucellsserial)||
||	[ &bull; scan_rslt_on_column](#function-servicescan_rslt_on_column)|[UCCellsSerial](#typedef-uccellsserial)||
||	[ &bull; scan_rslt_on_key](#function-servicescan_rslt_on_key)|[CCells](#typedef-ccells)||
||	[ &bull; scan_rslt_on_fraction](#function-servicescan_rslt_on_fraction)|[KCells](#typedef-kcells)||
||	[ &bull; scan_rslt_on](#function-servicescan_rslt_on)|[CompactResults](#typedef-compactresults)||
|||[Exception](#exception-exception)||
|||[Schema](#struct-schema)||
|||[SchemaPattern](#struct-schemapattern)||
|||[SpecSchemas](#struct-specschemas)||
|||[SpecFlags](#struct-specflags)||
|||[SpecFraction](#struct-specfraction)||
|||[SpecTimestamp](#struct-spectimestamp)||
|||[SpecKeyInterval](#struct-speckeyinterval)||
|||[SpecValue](#struct-specvalue)||
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
|```DELETE```|```5```|Delete Column Function |
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

### Enumeration: SpecFlagsOpt
The Scan options Flags Specifications for the SpecFlags 'options' bit 

|Name|Value|Description|
|---|---|---|
|```NONE```|```0```|No Flag Applied |
|```LIMIT_BY_KEYS```|```1```|Cells Limit by Keys |
|```OFFSET_BY_KEYS```|```4```|Cells Offset by Keys |
|```ONLY_KEYS```|```8```|Select Cells Only Keys without Value data |
|```ONLY_DELETES```|```10```|Select Cells Only with DELETE(cell-flag) |

### Enumeration: Flag
The Cell Flag 

|Name|Value|Description|
|---|---|---|
|```NONE```|```0```|Unknown/Undefined |
|```INSERT```|```1```|The Cell is an insert |
|```DELETE```|```2```|The Cell is a delete |
|```DELETE_VERSION```|```3```|The Cell is a delete-version |

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
|3|col_seq|[```KeySeq```](#enumeration-keyseq)|Column Key Sequence |optional||
|4|col_type|[```ColumnType```](#enumeration-columntype)|Column Type |optional||
|5|cell_versions|```i32```|Cell Versions |optional||
|6|cell_ttl|```i32```|Cell Time to Live |optional||
|7|blk_encoding|[```EncodingType```](#enumeration-encodingtype)|Block Encoding |optional||
|8|blk_size|```i32```|Block Size in Bytes |optional||
|9|blk_cells|```i32```|Number of Cells in Block |optional||
|10|cs_replication|```i8```|CellStore file Replication |optional||
|11|cs_size|```i32```|CellStore Size in Bytes |optional||
|12|cs_max|```i8```|Max CellStores in a Range |optional||
|13|log_rollout_ratio|```i8```|Write Fragment File on ratio reached |optional||
|14|log_compact_cointervaling|```i8```|Min. Cointervaling Fragments for Compaction |optional||
|15|log_fragment_preload|```i8```|Number of Fragment to Preload |optional||
|16|compact_percent|```i8```|Compact at percent reach |optional||
|17|revision|```i64```|Schema's revision/id |optional||

### Struct: SchemaPattern
The Schema Matching Pattern for the SpecSchema patterns 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|comp|[```Comp```](#enumeration-comp)|Logical comparator to Apply |default||
|2|value|```string```|The patern value to match against schema's column name |default||

### Struct: SpecSchemas
The Specs for Schemas for using with list_columns or compact_columns 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|cids|list&lt;```i64```&gt;|The Column IDs |default||
|2|names|list&lt;```string```&gt;|The Column Names |default||
|3|patterns|list&lt;[```SchemaPattern```](#struct-schemapattern)&gt;|The Schema's Column Name patterns |default||

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

### Struct: SpecInterval
The Cells Interval Specifications with interval-scope Flags 

| Key | Field | Type | Description | Requiredness | Default value |
| --- | --- | --- | --- | --- | --- |
|1|range_begin|[```Key```](#typedef-key)|Begin of Ranges evaluation with this Key inclusive |default||
|2|range_end|[```Key```](#typedef-key)|End of Ranges evaluation with this Key inclusive |default||
|3|range_offset|[```Key```](#typedef-key)|Offset of Ranges evaluation with this Key inclusive |default||
|4|offset_key|[```Key```](#typedef-key)|Offset Cell Key of a Scan, select cells from this key inclusive |default||
|5|offset_rev|```i64```|Offset Cell Timestamp of a Scan, select cells after this timestamp |optional||
|6|key_intervals|[```SpecKeyIntervals```](#typedef-speckeyintervals)|The Key Intervals |default||
|7|values|[```SpecValues```](#typedef-specvalues)|The Cell Value Specifications, cell-value match |default||
|8|ts_start|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Start Spec, the start of cells-interval timestamp match |optional||
|9|ts_finish|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Finish Spec, the finish of cells-interval timestamp match |optional||
|10|flags|[```SpecFlags```](#struct-specflags)|The Interval Flags Specification |optional||

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
|3|range_offset|[```Key```](#typedef-key)|Offset of Ranges evaluation with this Key inclusive |default||
|4|offset_key|[```Key```](#typedef-key)|Offset Cell Key of a Scan, select cells from this key inclusive |default||
|5|offset_rev|```i64```|Offset Cell Timestamp of a Scan, select cells after this timestamp |optional||
|6|key_intervals|[```SpecKeyIntervals```](#typedef-speckeyintervals)|The Key Intervals |default||
|7|values|[```SpecValuesSerial```](#typedef-specvaluesserial)|The Serial Cell Value Specifications, cell-value fields match |default||
|8|ts_start|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Start Spec, the start of cells-interval timestamp match |optional||
|9|ts_finish|[```SpecTimestamp```](#struct-spectimestamp)|The Timestamp Finish Spec, the finish of cells-interval timestamp match |optional||
|10|flags|[```SpecFlags```](#struct-specflags)|The Interval Flags Specification |optional||

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





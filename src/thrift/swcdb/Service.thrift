/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


/*
 * namespace for target languages
 */

/* The SWC-DB Thrift-Protocol C++ namespace 'SWC::Thrift' */
namespace cpp     SWC.Thrift

/* The SWC-DB Thrift-Protocol Python Module 'swcdb.thrift' */
namespace py      swcdb.thrift.gen

/* The SWC-DB Thrift-Protocol Ruby Module 'swcdb.thrift' */
namespace rb      swcdb.thrift.gen

/* The SWC-DB Thrift-Protocol C-GLIB prefix 'swcdb_thrift' */
namespace c_glib  swcdb_thrift

/* The SWC-DB Thrift-Protocol Java package 'org.swcdb.thrift.gen' */
namespace java    org.swcdb.thrift.gen

/*
namespace perl  SWC.thrift
namespace php   SWC
namespace rb    SWC.thrift
*/




/**
The SWC::Thrift::Exception a base for any Exceptions
both for the Thrift-Protocol and SWC-DB Errors. */
exception Exception {
  /** The corresponding Thrift-Procotol or SWC-DB Error Code */
  1: i32 code

  /** The message describing the error code */
  2: string message
}



/** The TIMESTAMP NULL value */
const i64 TIMESTAMP_NULL = -9223372036854775807

/** The TIMESTAMP AUTO value */
const i64 TIMESTAMP_AUTO = -9223372036854775806

/** The COUNTER equal operation */
const i8  COUNTER_OP_EQUAL = 1



/** Column Key Sequences */
enum KeySeq {
  /** Unknown/Unrecognized Type */
  UNKNOWN       = 0,

  /** The Lexical Key Order Sequence */
  LEXIC         = 1,

  /** The Volumetric Key Order Sequence */
  VOLUME        = 2,

  /** The by Fractions Count on Lexical Key Order Sequence */
  FC_LEXIC      = 3,

  /** The by Fractions Count on Volumetric Key Order Sequence */
  FC_VOLUME     = 4

}



/** Column Value Types */
enum ColumnType {
  /** Unknown/Unrecognized Type */
  UNKNOWN       = 0,

  /** A Plain Column Value */
  PLAIN         = 1,

  /** A Counter Column Value with integrity of signed-64bit */
  COUNTER_I64   = 2,

  /** A Counter Column Value with integrity of signed-32bit */
  COUNTER_I32   = 3,

  /** A Counter Column Value with integrity of signed-16bit */
  COUNTER_I16   = 4,

  /** A Counter Column Value with integrity of signed-8bit */
  COUNTER_I8    = 5,

  /** A Serial Column Value */
  SERIAL        = 6,

  /** Not used - experimental */
  CELL_DEFINED  = 15
}



/** Data Encoding Types */
enum EncodingType {
  /** Encoding by Ranger DEFAULT configurations */
  DEFAULT = 0,

  /** No Encoding */
  PLAIN   = 1,

  /** Encode with zlib */
  ZLIB    = 2,

  /** Encode with snappy */
  SNAPPY  = 3,

  /** Encode with zstandard */
  ZSTD  = 4,

  /** Unrecognized Type */
  UNKNOWN = 255
}



/** The Schema Definition */
struct Schema {

  /** Column ID */
  1: optional i64           cid

  /** Column Name */
  2: optional string        col_name

  /** Column Tags */
  3: list<string>           col_tags

  /** Column Key Sequence */
  4: optional KeySeq        col_seq

  /** Column Type */
  5: optional ColumnType    col_type


  /** Cell Versions */
  6: optional i32           cell_versions

  /** Cell Time to Live */
  7: optional i32           cell_ttl


  /** Block Encoding */
  8: optional EncodingType  blk_encoding

  /** Block Size in Bytes */
  9: optional i32           blk_size

  /** Number of Cells in Block */
  10: optional i32          blk_cells


  /** CellStore file Replication */
  11: optional i8           cs_replication

  /** CellStore Size in Bytes */
  12: optional i32          cs_size

  /** Max CellStores in a Range */
  13: optional i8           cs_max


  /** Write Fragment File on ratio reached */
  14: optional i8           log_rollout_ratio

  /** Min. Cointervaling Fragments for Compaction */
  15: optional i8           log_compact_cointervaling

  /** Number of Fragment to Preload */
  16: optional i8           log_fragment_preload


  /** Compact at percent reach */
  17: optional i8           compact_percent


  /** Schema's revision/id */
  18: optional i64          revision
}

/** A list-container of Schemas */
typedef list<Schema> Schemas



/** A Cell Key defined as binary(bytes) items in a list-container */
typedef list<binary> Key



/** Manage Columns schema function Flags */
enum SchemaFunc {
  /** Create Column Function */
  CREATE                = 3,

  /** Delete Column Function */
  REMOVE                = 5,

  /** Modify Column Function */
  MODIFY                = 7
}



/** The available logical Comparators, plus extended logic options applied with 'v' for VOLUME */
enum Comp {
  /** [         ]  :   none               (no comparison applied) */
  NONE = 0x0,

  /** [  =^     ]  :   -pf [prefix]       (starts-with) */
  PF   = 0x1,

  /** [ &gt;    ]  :   -gt                (greater-than) */
  GT   = 0x2,

  /** [ &gt;=   ]  :   -ge                (greater-equal) */
  GE   = 0x3,

  /** [  =      ]  :   -eq                (equal) */
  EQ   = 0x4,

  /** [ &lt;=   ]  :   -le                (lower-equal) */
  LE   = 0x5,

  /** [ &lt;    ]  :   -lt                (lower-than) */
  LT   = 0x6,

  /** [  !=     ]  :   -ne                (not-equal) */
  NE   = 0x7,

  /** [  re     ]  :   -re [r,regexp]     (regular-expression) */
  RE   = 0x8,

  /** [ v&gt;   ]  :   -vgt               (vol greater-than) */
  VGT  = 0x9,

  /** [ v&gt;=  ]  :   -vge               (vol greater-equal) */
  VGE  = 0xA,

  /** [ v&lt;=  ]  :   -vle               (vol lower-equal) */
  VLE  = 0xB,

  /** [ v&lt;   ]  :   -vlt               (vol lower-than) */
  VLT  = 0xC,

  /** [ %&gt;   ]  :   -subset [sbs]      (subset) */
  SBS     = 0x0D,

  /** [ &lt;%   ]  :   -supset [sps]      (superset) */
  SPS     = 0x0E,

  /** [ ~&gt;   ]  :   -posubset [posbs]  (eq/part ordered subset) */
  POSBS   = 0x0F,

  /** [ &lt;~   ]  :   -posupset [posps]  (eq/part ordered superset) */
  POSPS   = 0x10,

  /** [ -&gt;   ]  :   -fosubset [fosbs]  (eq/full ordered subset) */
  FOSBS   = 0x11,

  /** [ &lt;-   ]  :   -fosupset [fosps]  (eq/full ordered superset) */
  FOSPS   = 0x12,

  /** [ :&lt;   ]  :   -fip  (fraction include prior) */
  FIP     = 0x13,

  /** [ :       ]  :   -fi   (fraction include) */
  FI      = 0x14,

}


/** The Schema matching Pattern */
struct SchemaPattern {
  /** Logical comparator to Apply */
  1: Comp   comp

  /** The patern value to match against */
  2: string value
}

/** The Schema Tags patterns for the SchemaPatterns */
struct SchemaTagsPatterns {
  /** Logical comparator to Apply, unsupported PF, RE and Vol. kind */
  1: Comp   comp

  /** The tags patterns to match against schema's column tags */
  2: list<SchemaPattern>   values
}


/** The Schema Patterns for the SpecSchemas */
struct SchemaPatterns {
  /** The Schema patterns for selecting by Column Name */
  1: list<SchemaPattern>   names

  /** The Schema patterns for selecting by Column Tags */
  2: SchemaTagsPatterns    tags
}


/** The Specs for Schemas for using with list_columns or compact_columns */
struct SpecSchemas {
  /** The Column IDs */
  1: list<i64>             cids

  /** The Column Names */
  2: list<string>          names

  /** The Schema's selector patterns */
  3: SchemaPatterns        patterns
}



/** The Scan options Flags Specifications for the SpecFlags 'options' bit */
enum SpecFlagsOpt {
  /** No Flag Applied */
  NONE              = 0,

  /** Cells Limit by Keys */
  LIMIT_BY_KEYS     = 1,

  /** Cells Offset by Keys */
  OFFSET_BY_KEYS    = 4,

  /** Select Cells Only Keys without Value data */
  ONLY_KEYS         = 8,

  /** Select Cells Only with DELETE(cell-flag) */
  ONLY_DELETES      = 10
}

/** The Scan Specifications Flags */
struct SpecFlags {
  /** Limit to this number of cells */
  1: optional i64     limit

  /** Scan from this number of cells Offset on matching Cell-Interval */
  2: optional i64     offset

  /** Select only this number of Versions of a given Cell-Key */
  3: optional i32     max_versions

  /** return results with reach of this Buffer size in bytes */
  4: optional i32     max_buffer

  /** The options bit by SpecFlagsOpt */
  5: optional i8      options
}



/* THE SPEC INTERVAL UPDATE VALUE BY COLUMN VALUE TYPE */

enum UpdateOP {
  /** The OP supported by column-types: PLAIN, SERIAL, COUNTER. Replaces with the update value (_default as well if other OP not supported by the col-type_) */
  REPLACE     = 0x00,

  /** The OP supported by column-types: PLAIN, SERIAL. Appends the update value to the cell's current  */
  APPEND      = 0x01,

  /** The OP supported by column-types: PLAIN, SERIAL. Prepends the update value to the cell's current */
  PREPEND     = 0x02,

  /** The OP supported by column-type PLAIN. Inserts the update value at position in current value (appends if pos above value) */
  INSERT      = 0x03,

  /** The OP supported by column-type PLAIN. Overwrites the current value at position with new value (appends if pos above value) */
  OVERWRITE   = 0x04,

  /** The OP supported by column-type SERIAL. update is done by the inner serial-fields defintions */
  SERIAL      = 0x05,
}

struct SpecUpdateOP {
  /** The Operation of update */
  1: UpdateOP       op

  /** The position/index of INSERT and OVERWRITE update operations */
  2: optional i32   pos
}


/** The Value specs for an Updating Interval of 'updating' in SpecIntervalPlain */
struct SpecIntervalUpdatePlain {

  /** The bytes value for the updated cell */
  1: binary                 v

  /** The timestamp for the updated cell NULL: MIN_INT64+1, AUTO:MIN_INT64+2 (or not-set) */
  2: optional i64           ts

  /** Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD */
  3: optional EncodingType  encoder

  /** Optionally the operaton of value update */
  4: optional SpecUpdateOP  update_op
}

/** The Value specs for an Updating Interval of 'updating' in SpecIntervalCounter */
struct SpecIntervalUpdateCounter {

  /** The int64 value for the updated cell */
  1: i64                    v
  
  /** The Opration pf Counter, available: COUNTER_OP_EQUAL */
  2: i64                    op = 0

  /** The timestamp for the updated cell NULL: MIN_INT64+1, AUTO:MIN_INT64+2 (or not-set) */
  3: optional i64           ts

  /** Optionally the operaton of value update */
  4: optional SpecUpdateOP  update_op
}

/** The Value specs for an Updating Interval of 'updating' in SpecIntervalSerial */
struct SpecIntervalUpdateSerial {
  /** The timestamp for the updated cell NULL: MIN_INT64-1, AUTO:MIN_INT64-1 */
  1: i64                    ts

  /** The values of serial-fields for the updated cell */
  2: CellValuesSerial       v

  /** The values of serial-fields for the the SERIAL operation update */
  3: CellValuesSerialOp     v_op

  /** Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD */
  4: optional EncodingType  encoder

  /** Optionally the operaton of value update */
  5: optional SpecUpdateOP  update_op
}



/* THE SPEC VALUE BY COLUMN VALUE TYPE */

/** The Specifications of INT64 Serial Value Field */
struct SpecValueSerial_INT64 {
  /** Logical comparator to Apply */
  1: Comp     comp

  /** The int64 to match against the value field */
  2: i64      v
}

/** The Specifications of DOUBLE Serial Value Field */
struct SpecValueSerial_DOUBLE {
  /** Logical comparator to Apply */
  1: Comp     comp

  /** The double to match against the value field */
  2: double   v
}

/** The Specifications of BYTES Serial Value Field */
struct SpecValueSerial_BYTES {
  /** Logical comparator to Apply */
  1: Comp     comp

  /** The binary(bytes) to match against the value field */
  2: binary   v
}

/** The Specifications of KEY Serial Value Field */
struct SpecValueSerial_KEY {
  /** The Key Sequence to use */
  1: KeySeq   seq

  /** The Specification of the Key to match against the value field */
  2: SpecKey  v
}

/** The Specifications of LIST_INT64(LI) Serial Value Field */
struct SpecValueSerial_LI {
  /** Logical comparator to Apply */
  1: Comp                         comp

  /** The List of Int64 to match against the value field */
  2: list<SpecValueSerial_INT64>  v
}

/** The Specifications of LIST_BYTES(LB) Serial Value Field */
struct SpecValueSerial_LB {
  /** Logical comparator to Apply */
  1: Comp                         comp

  /** The List of Bytes to match against the value field */
  2: list<SpecValueSerial_BYTES>  v
}

struct SpecValueSerialField {
  /** The Field Id of the Value Field */
  1: i32                              field_id
  /** The specifications of Int64 for the field */
  2: optional SpecValueSerial_INT64   spec_int64
  /** The specifications of Double for the field */
  3: optional SpecValueSerial_DOUBLE  spec_double
  /** The specifications of Bytes for the field */
  4: SpecValueSerial_BYTES            spec_bytes
  /** The specifications of Cell-Key for the field */
  5: SpecValueSerial_KEY              spec_key
  /** The specifications of List Int64 for the field */
  6: SpecValueSerial_LI               spec_li
  /** The specifications of List Bytes for the field */
  7: SpecValueSerial_LB               spec_lb
}
/** The Serial Cell Value Specifications defined as SpecValueSerialField items in a list-container */
typedef list<SpecValueSerialField> SpecValueSerialFields


/** The Plain Value Specifications, option to use with Extended Logical Comparators  */
struct SpecValuePlain {
  /** Logical comparator to Apply */
  1: Comp   comp

  /** The binary(bytes) to match against the Cell value */
  2: binary v
}
/** The Cell Value Specifications defined as SpecValuePlain items in a list-container */
typedef list<SpecValuePlain> SpecValuesPlain

/** The Counter Value Specifications, option to use with Extended Logical Comparators  */
struct SpecValueCounter {
  /** Logical comparator to Apply */
  1: Comp   comp

  /** The int64 to match against the Cell value */
  2: i64    v
}
/** The Cell Value Specifications defined as SpecValueCounter items in a list-container */
typedef list<SpecValueCounter> SpecValuesCounter

/** The Serial Value Specifications */
struct SpecValueSerial {
  /** Logical comparator to Apply */
  1: Comp                   comp

  /** The Serial Value Specifications to match against the SERIAL Cell value fields */
  2: SpecValueSerialFields  fields
}
/** The Cell Value Specifications defined as SpecValueSerial items in a list-container */
typedef list<SpecValueSerial> SpecValuesSerial



/* THE SPEC INTERVAL BY COLUMN VALUE TYPE */

/** The Fraction Specifications */
struct SpecFraction {
  /** Logical comparator to Apply */
  1: Comp   comp
  /** The binary(bytes) to match against a fraction of a Cell-Key*/
  2: binary f
}
/** The Key Specifications defined as SpecFraction items in a list-container */
typedef list<SpecFraction> SpecKey

/** The Timestamp Specifications */
struct SpecTimestamp {
  /** Logical comparator to Apply */
  1: Comp comp

  /** The timestamp in nanoseconds to match against the Cell timestamp/version (not the revision) */
  2: i64  ts
}

/** The Key Interval Specifications */
struct SpecKeyInterval {
  /** The Key Start Spec, the start of cells-interval key match */
  1: SpecKey  start

  /** The Key Finish Spec, the finish of cells-interval key match */
  2: SpecKey  finish
}
/** The Key Intervals Specifications defined as SpecKeyInterval items in a list-container */
typedef list<SpecKeyInterval> SpecKeyIntervals

/** The Scan Interval Specs Options for the SpecIntervalPlain, SpecIntervalCounter and SpecIntervalSerial 'options' bit */
enum SpecIntervalOptions {
  /** Update Bit Option */
  UPDATING      = 4,

  /** Delete Bit Option */
  DELETING      = 8,
}


/** The Cells Interval Plain type Specifications with interval-scope Flags */
struct SpecIntervalPlain {
  /** Begin of Ranges evaluation with this Key inclusive */
  1: Key                            range_begin

  /** End of Ranges evaluation with this Key inclusive */
  2: Key                            range_end

  /** Offset Cell Key of a Scan, select cells from this key inclusive */
  3: Key                            offset_key

  /** Offset Cell Timestamp of a Scan, select cells after this timestamp  */
  4: optional i64                   offset_rev

  /** The Key Intervals */
  5: SpecKeyIntervals               key_intervals

  /** The Cell Value Specifications, cell-value match for plain type */
  6: SpecValuesPlain                values

  /** The Timestamp Start Spec, the start of cells-interval timestamp match */
  7: optional SpecTimestamp         ts_start

  /** The Timestamp Finish Spec, the finish of cells-interval timestamp match */
  8: optional SpecTimestamp         ts_finish

  /** The Interval Flags Specification */
  9: optional SpecFlags             flags

  /** The Interval Options Specification */
  10: optional SpecIntervalOptions  options

  /** The Value spec of an Updating Interval */
  11: optional SpecIntervalUpdatePlain   updating
}

/** The Cells Interval Counter type Specifications with interval-scope Flags */
struct SpecIntervalCounter {
  /** Begin of Ranges evaluation with this Key inclusive */
  1: Key                                    range_begin

  /** End of Ranges evaluation with this Key inclusive */
  2: Key                                    range_end

  /** Offset Cell Key of a Scan, select cells from this key inclusive */
  3: Key                                    offset_key

  /** Offset Cell Timestamp of a Scan, select cells after this timestamp  */
  4: optional i64                           offset_rev

  /** The Key Intervals */
  5: SpecKeyIntervals                       key_intervals

  /** The Cell Value Specifications, cell-value match for counter type */
  6: SpecValuesCounter                      values

  /** The Timestamp Start Spec, the start of cells-interval timestamp match */
  7: optional SpecTimestamp                 ts_start

  /** The Timestamp Finish Spec, the finish of cells-interval timestamp match */
  8: optional SpecTimestamp                 ts_finish

  /** The Interval Flags Specification */
  9: optional SpecFlags                     flags

  /** The Interval Options Specification */
  10: optional SpecIntervalOptions          options

  /** The Value spec of an Updating Interval */
  11: optional SpecIntervalUpdateCounter    updating
}

/** The Cells Interval Serial type Specifications with interval-scope Flags */
struct SpecIntervalSerial {
  /** Begin of Ranges evaluation with this Key inclusive */
  1: Key                                  range_begin

  /** End of Ranges evaluation with this Key inclusive */
  2: Key                                  range_end

  /** Offset Cell Key of a Scan, select cells from this key inclusive */
  3: Key                                  offset_key

  /** Offset Cell Timestamp of a Scan, select cells after this timestamp  */
  4: optional i64                         offset_rev

  /** The Key Intervals */
  5: SpecKeyIntervals                     key_intervals

  /** The Serial Cell Value Specifications, cell-value fields match */
  6: SpecValuesSerial                     values

  /** The Timestamp Start Spec, the start of cells-interval timestamp match */
  7: optional SpecTimestamp               ts_start

  /** The Timestamp Finish Spec, the finish of cells-interval timestamp match */
  8: optional SpecTimestamp               ts_finish

  /** The Interval Flags Specification */
  9: optional SpecFlags                   flags

  /** The Interval Options Specification */
  10: optional SpecIntervalOptions        options

  /** The Serial-Value spec of an Updating Interval */
  11: optional SpecIntervalUpdateSerial   updating
}



/* THE SPEC COLUMN BY COLUMN VALUE TYPE */

/** The Column Specifications, the Cells-Intervals(SpecIntervalPlain/s) specification for a PLAIN Type column */
struct SpecColumnPlain {
  /** The Column ID */
  1: i64                cid

  /** The Cells Interval in a list-container*/
  2: list<SpecIntervalPlain> intervals
}

/** The Column Specifications, the Cells-Intervals(SpecIntervalCounter/s) specification for a COUNTER Type column */
struct SpecColumnCounter {
  /** The Column ID */
  1: i64                cid

  /** The Cells Interval in a list-container*/
  2: list<SpecIntervalCounter> intervals
}

/** The Column Specifications, the Cells-Intervals(SpecIntervalSerial/s) specification for a SERIAL Type Column */
struct SpecColumnSerial {
  /** The Column ID */
  1: i64                      cid

  /** The Serial Cells Interval in a list-container*/
  2: list<SpecIntervalSerial> intervals
}



/** The Scan Specifications, the Columns-Intervals(SpecColumn/s) with global-scope Flags */
struct SpecScan {
  /** The Plain Column Intervals(SpecColumnPlain) in a list-container */
  1: list<SpecColumnPlain>    columns_plain

  /** The Counter Column Intervals(SpecColumnCounter) in a list-container */
  2: list<SpecColumnCounter>  columns_counter

  /** The Serial Column Intervals(SpecColumnSerial) in a list-container */
  3: list<SpecColumnSerial>   columns_serial

  /** The Global Flags Specification */
  4: optional SpecFlags     flags
}



/** The Cell Flag */
enum Flag {
  /** Unknown/Undefined */
  NONE            = 0,

  /** The Cell is an insert */
  INSERT          = 1,

  /** The Cell is a delete versions lower-equal */
  DELETE_LE       = 2,

  /** The Cell is a  delete version equal */
  DELETE_EQ       = 3
}



/** The Cell data for using with Update of PLAIN Column Type */
struct UCellPlain {
  /** The Cell Flag */
  1: Flag                   f

  /** The Cell Key */
  2: Key                    k

  /** The Cell Timestamp in nanoseconds */
  3: optional i64           ts

  /** The Cell Version is in timestamp descending */
  4: optional bool          ts_desc

  /** The Cell Value */
  5: binary                 v

  /** Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD */
  6: optional EncodingType  encoder
}

/** The Cells for Update defined as UCellPlain items in a list-container */
typedef list<UCellPlain> UCellsPlain

/** The Cells for Update for a Column Id defined as UCellsPlain items in a map-container by CID */
typedef map<i64, UCellsPlain> UCCellsPlain


/** The Cell data for using with Update of COUNTER Column Type */
struct UCellCounter {
  /** The Cell Flag */
  1: Flag                   f

  /** The Cell Key */
  2: Key                    k

  /** The Cell Timestamp in nanoseconds */
  3: optional i64           ts

  /** The Cell Version is in timestamp descending */
  4: optional bool          ts_desc

  /** The Cell Counter Operation */
  5: i8                     op = 0

  /** The Cell Counter Value */
  6: i64                    v = 0
}

/** The Cells for Update defined as UCellCounter items in a list-container */
typedef list<UCellCounter> UCellsCounter

/** The Cells for Update for a Column Id defined as UCellsCounter items in a map-container by CID */
typedef map<i64, UCellsCounter> UCCellsCounter



/** The Serial Value Cell field */
struct CellValueSerial {
  /** The Field ID, a single ID can have any/all the field types */
  1: i32                      field_id

  /** The INT64 type field */
  2: optional i64             v_int64

  /** The DOUBLE type field */
  3: optional double          v_double

  /** The BYTES type field */
  4: optional binary          v_bytes

  /** The Cell KEY type field */
  5: optional Key             v_key

  /** The LIST INT64 type field */
  6: optional list<i64>       v_li

  /** The LIST BYTES type field */
  7: optional list<binary>    v_lb
}
/** The Serial Cell Value Fields defined as CellValueSerial items in a list-container */
typedef list<CellValueSerial> CellValuesSerial
//



/** A control bit of default-state */
const i8 FU_CTRL_DEFAULT       = 0
/** A control bit to not add a new field in case a field for update does not exist (Except for BY_INDEX OP) */
const i8 FU_CTRL_NO_ADD_FIELD  = 1
/** A control bit to delete the given field */
const i8 FU_CTRL_DELETE_FIELD  = 2
/**  A control bit to add/set if not exists (only available with OP used by BY_UNIQUE OR BY_COND in List field-types ) */
const i8 FU_CTRL_VALUE_SET     = 4
/**  A control bit delete any that exist (only available with OP used by BY_UNIQUE OR BY_COND in List field-types ) */
const i8 FU_CTRL_VALUE_DEL     = 8


/** MATH Operations for Serial Field Update of types INT64 and DOUBLE */
enum FU_MATH_OP {
  /** set field value to the new value */
  EQUAL           = 0,

  /** plus new value to field's value (negative number allowed) */
  PLUS            = 1,

  /** multiply current value by update value */
  MULTIPLY        = 2,

  /** divide current value by the new value (ignored at zero) */
  DIVIDE          = 3
}

/** LIST Operations for Serial Field Update of array/list/bytes with LIST-op in the inner SERIAL fields */
enum FU_LIST_OP {
  /** Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Replaces with the update value */
  REPLACE         = 0,

  /** Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Appends the update value to a field value  */
  APPEND          = 1,

  /** Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Prepends the update value to a field value */
  PREPEND         = 2,

  /** Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Insert the update value at position in a field value (appends if pos above value) */
  INSERT          = 3,

  /** Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Overwrites a field value at position with new value (appends if pos above value) */
  OVERWRITE       = 4,

  /** Supported by field-types: BYTES, LIST_BYTES, LIST_INT64. Erases the position in a field value */
  ERASE           = 5,

  /** Supported by field-types: LIST_BYTES, LIST_INT64. The field value items have CTRL_VALUE_SET/DEL OP */
  BY_UNIQUE       = 6,

  /** Supported by field-types: LIST_BYTES, LIST_INT64. The field value items have CTRL_VALUE_SET/DEL OP and Comparator */
  BY_COND         = 7,

  /** Supported by field-types: LIST_BYTES, LIST_INT64. The field value is with Postion and OP in items  */
  BY_INDEX        = 8
}



/** Serial INT64 Field Update */
struct FU_INT64 {
  1: i8                       ctrl = FU_CTRL_DEFAULT
  2: FU_MATH_OP               op = FU_MATH_OP.EQUAL
  3: optional i32             pos
  4: optional Comp            comp
  5: i64                      v
}

/** Serial DOUBLE Field Update */
struct FU_DOUBLE {
  1: i8                       ctrl = FU_CTRL_DEFAULT
  2: FU_MATH_OP               op = FU_MATH_OP.EQUAL
  3: optional i32             pos
  4: optional Comp            comp
  5: double                   v
}

/** Serial BYTES Field Update */
struct FU_BYTES {
  1: i8                       ctrl = FU_CTRL_DEFAULT
  2: FU_LIST_OP               op = FU_LIST_OP.REPLACE
  3: optional i32             pos
  4: optional Comp            comp
  5: binary                   v
}

/** Serial LIST_INT64 Field Update */
struct FU_LI {
  1: i8                       ctrl = FU_CTRL_DEFAULT
  2: FU_LIST_OP               op = FU_LIST_OP.REPLACE
  3: optional i32             pos
  4: list<FU_INT64>           v
}

/** Serial LIST_BYTES Field Update */
struct FU_LB {
  1: i8                       ctrl = FU_CTRL_DEFAULT
  2: FU_LIST_OP               op = FU_LIST_OP.REPLACE
  3: optional i32             pos
  4: list<FU_BYTES>           v
}


/** The Serial Values Cell field with Update Operation */
struct CellValueSerialOp {
  /** The Field ID, a single ID can have any/all the field types */
  1: i32                      field_id

  /** The INT64 type update-field */
  2: optional FU_INT64        v_int64

  /** The DOUBLE type update-field */
  3: optional FU_DOUBLE       v_double

  /** The BYTES type update-field */
  4: optional FU_BYTES        v_bytes

  /** The Cell KEY type update-field */
  5: optional Key             v_key

  /** The LIST INT64 type update-field */
  6: optional FU_LI           v_li

  /** The LIST BYTES type update-field */
  7: optional FU_LB           v_lb
}
/** The Serial Cell Value Fields defined as CellValueSerialOp items in a list-container */
typedef list<CellValueSerialOp> CellValuesSerialOp
//



/** The Cell data for using with Update of SERIAL Column Type */
struct UCellSerial {
  /** The Cell Flag */
  1: Flag                       f

  /** The Cell Key */
  2: Key                        k

  /** The Cell Timestamp in nanoseconds */
  3: optional i64               ts

  /** The Cell Version is in timestamp descending */
  4: optional bool              ts_desc

  /** The Serial Cell Value fields */
  5: CellValuesSerial           v

  /** Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD */
  6: optional EncodingType      encoder
}

/** The Cells for Update defined as UCellSerial items in a list-container */
typedef list<UCellSerial> UCellsSerial

/** The Cells for Update for a Column Id defined as UCellsSerial items in a map-container by CID */
typedef map<i64, UCellsSerial> UCCellsSerial



/** The Cell for results list of scan */
struct CellPlain {
  /** The Column Name */
  1: string           c

  /** The Cell Key */
  2: Key              k

  /** The Cell Timestamp */
  3: i64              ts

  /** The Cell Value */
  4: binary           v
}
/** A list-container of Plain Cells */
typedef list<CellPlain>   CellsPlain


/** The Counter Cell for results list of scan */
struct CellCounter {
  /** The Column Name */
  1: string           c

  /** The Cell Key */
  2: Key              k

  /** The Cell Timestamp */
  3: i64              ts

  /** The Cell Counter Value */
  4: i64              v

  /** The Counter EQ since ts */
  5: optional i64     eq
}
/** A list-container of Counter Cells */
typedef list<CellCounter> CellsCounter


/** The Serial Cell for results list of scan */
struct CellSerial {
  /** The Column Name */
  1: string           c

  /** The Cell Key */
  2: Key              k

  /** The Cell Timestamp */
  3: i64              ts

  /** The Cell Serial Value */
  4: CellValuesSerial v
}
/** A list-container of Serial Cells */
typedef list<CellSerial>  CellsSerial


/** The Cells for results list of scan */
struct Cells {
  /** The Cells, defined as Cell items in a list-container */
  1: CellsPlain     plain_cells

  /** The Cells, defined as Cell items in a list-container */
  2: CellsCounter   counter_cells

  /** The Serial Cells, defined as CellSerial items in a list-container */
  3: CellsSerial    serial_cells
}




/** The Plain column type Cell for results on Columns of scan */
struct CCellPlain {
  /** The Cell Key */
  1: Key              k

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Value */
  3: binary           v
}

/** The Counter column type Cell for results on Columns of scan */
struct CCellCounter {
  /** The Cell Key */
  1: Key              k

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Counter Value */
  3: i64              v

  /** The Counter EQ since ts */
  4: optional i64     eq
}

/** The Serial column type Cell for results on Columns of scan */
struct CCellSerial {
  /** The Cell Key */
  1: Key              k

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Serial Value */
  3: CellValuesSerial v
}

/** The Column Cells for results on Columns of scan */
struct ColCells {
  /** The Plain type Cells, defined as CCellPlain items in a list-container */
  1: list<CCellPlain>       plain_cells

  /** The Counter type Cells, defined as CCellCounter items in a list-container */
  2: list<CCellCounter>     counter_cells

  /** The Serial type Cells, defined as CCellSerial items in a list-container */
  3: list<CCellSerial>      serial_cells
}

/** The Columns Cells for results on Columns of scan, defined as ColCells items in a map-container by Column Name */
typedef map<string, ColCells> CCells




/** The Key Cell for results on Key of scan */
struct KCell {
  /** The Column Name */
  1: string           c

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Value */
  3: binary           v
}

/** The Key Serial Cell for results on Key of scan */
struct KCellSerial {
  /** The Column Name */
  1: string           c

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Serial Value */
  3: CellValuesSerial v
}

/** The Key Cells for results on Key of scan */
struct kCells {
  /** The Cell Key */
  1: Key                k

  /** The Key's Cells, defined as KCell items in a list-container */
  2: list<KCell>        cells

  /** The Key's Serial Cells, defined as KCellSerial items in a list-container */
  3: list<KCellSerial>  serial_cells
}

/** The Keys Cells for results on Key of scan, defined as kCells items in a list-container */
typedef list<kCells> KCells




/** The Fraction Cell for results on Fraction of scan */
struct FCell {
  /** The Column Name */
  1: string           c

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Value */
  3: binary           v
}

/** The Fraction Serial Cell for results on Fraction of scan */
struct FCellSerial {
  /** The Column Name */
  1: string           c

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Serial Value */
  3: CellValuesSerial v
}

/** The Fraction Cells for results on Fraction of scan */
struct FCells {

  /** The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes */
  1: map<binary, FCells>  f

  /** The current Fraction's Cells, defined as FCell items in a list-container */
  2: list<FCell>          cells

  /** The current Fraction's Serial Cells, defined as FCellSerial items in a list-container */
  3: list<FCellSerial>    serial_cells

}





/** A Grouped Cells result for results of scan, determined by the request's CellsResult enum */
struct CellsGroup {
  /** The Cells in a list, defined as Cell items in a list-container */
  1: Cells   cells

  /** The Columns Cells in a map-container, defined as ColCells items by Column Name */
  2: CCells  ccells

  /** The Keys Cells in a list, defined as kCells items in a list-container */
  3: KCells  kcells

  /** The Fraction Cells in struct FCells */
  4: FCells  fcells
}


/** The Cells Results types for using with CellsGroup requests */
enum CellsResult {
  /** Correspond to result on Cells (Cells in list) */
  IN_LIST     = 0,

  /** Correspond to result on CCells (Columns Cells) */
  ON_COLUMN   = 1,

  /** Correspond to result on KCells (Keys Cells) */
  ON_KEY      = 2,

  /** Correspond to result on FCells (Fraction Cells) */
  ON_FRACTION = 3
}



/** The Compact Result  */
struct CompactResult {
  /** Column ID */
  1: i64 cid

  /** Error */
  2: i32 err
}

/** The Compact Results, defined as CompactResult items in a list-container */
typedef list<CompactResult> CompactResults



/** The Result of 'exec_sql' */
struct Result {
  /** Set with result for 'list columns' query */
  1: Schemas        schemas

  /** Set with result for 'select' query */
  2: Cells          cells

  /** Set with result for 'compact columns' query */
  3: CompactResults compact
}



/** The SWC-DB Thrift Service */
service Service {


  /**
    * The direct SQL method to Manage Column.
    */
  void sql_mng_column(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),


  /**
    * The direct SQL method to List Columns
    */
  Schemas sql_list_columns(

    /** The SQL string to Execute */
    1:string sql

  )  throws (1:Exception e),


  /**
    * The direct SQL method to Compact Columns
    */
  CompactResults sql_compact_columns(

    /** The SQL string to Execute */
    1:string sql

  )  throws (1:Exception e),


  /** The direct SQL method to select cells with result in Cells List. */
  Cells sql_select(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),



  /** The direct SQL method to select cells with result in CellsPlain. */
  CellsPlain sql_select_plain(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),


  /** The direct SQL method to select cells with result in CellsCounter. */
  CellsCounter sql_select_counter(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),


  /** The direct SQL method to select cells with result in CellsSerial. */
  CellsSerial sql_select_serial(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),



  /** The direct SQL method to select cells with result in Columns Cells map. */
  CCells sql_select_rslt_on_column(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),


  /** The direct SQL method to select cells with result in Key Cells list. */
  KCells sql_select_rslt_on_key(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),


  /** The direct SQL method to select cells with result in Fractons Cells. */
  FCells sql_select_rslt_on_fraction(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),


  /** The SQL method to select cells with result set by the request's type of CellsResult. */
  CellsGroup  sql_query(

    /** The SQL string to Execute */
    1:string sql,

    /** The Type of Cells Result for the response */
    2:CellsResult rslt

  ) throws (1:Exception e),


  /** The direct SQL method to update cells optionally to work with updater-id. */
  void sql_update(

    /** The SQL string to Execute */
    1:string sql,

    /** The Updater ID to work with */
    2:i64 updater_id = 0

  ) throws (1:Exception e),


  /** The SQL method to execute any query. */
  Result exec_sql(

    /** The SQL string to Execute */
    1:string sql

  ) throws (1:Exception e),



  /** The method to Create an Updater ID with buffering size in bytes. */
  i64 updater_create(

    /** The buffer size of the Updater */
    1:i32 buffer_size

  ) throws (1:Exception e),


  /** The method to Close an Updater ID. */
  void updater_close(

    /** The Updater ID to close */
    1:i64 id

  ) throws (1:Exception e),



  /** The direct method to update cells with cell in Update-Columns-Cells-Plain,
    * optionally to work with updater-id.
    */
  void update_plain(

    /** The Cells to update  */
    1:UCCellsPlain cells,

    /** The Updater ID to use for write */
    2:i64 updater_id = 0

  ) throws (1:Exception e),


  /** The direct method to update cells with cell in Update-Columns-Cells-Counter,
    * optionally to work with updater-id.
    */
  void update_counter(

    /** The Counter Cells to update  */
    1:UCCellsCounter cells,

    /** The Updater ID to use for write */
    2:i64 updater_id = 0

  ) throws (1:Exception e),


  /** The direct method to update cells with cell in Update-Columns-Cells-Serial,
    * optionally to work with updater-id.
    */
  void update_serial(

    /** The Serial Cells to update  */
    1:UCCellsSerial cells,

    /** The Updater ID to use for write */
    2:i64 updater_id = 0

  ) throws (1:Exception e),


  /** The method is to update cells by several Column-Types,
    * optionally to work with updater-id.
    */
  void update_by_types(

    /** The PLAIN Cells to update  */
    1:UCCellsPlain    plain,

    /** The COUNTER Cells to update  */
    2:UCCellsCounter  counter,

    /** The SERIAL Cells to update  */
    3:UCCellsSerial   serial,

    /** The Updater ID to use for write */
    4:i64 updater_id = 0

  ) throws (1:Exception e),



  /** The direct method to Manage Column  */
  void  mng_column(

    /** The Action Function to use */
    1:SchemaFunc func,

    /** The Schema for the Action */
    2:Schema schema

  ) throws (1:Exception e),


  /** The direct method to List Columns  */
  Schemas list_columns(

    /** The Schemas Specifications to match Schema for response */
    1:SpecSchemas spec

  ) throws (1:Exception e),


  /** The direct method to Compact Columns  */
  CompactResults compact_columns(

    /** The Schemas Specifications to match columns to Compact */
    1:SpecSchemas spec

  )throws (1:Exception e),


  /** The direct method to select cells with result in Cells List. */
  Cells scan(

    /** The Scan Specifications for the scan */
    1:SpecScan spec

  ) throws (1:Exception e),


  /** The direct method to select cells with result in Columns Cells map. */
  CCells scan_rslt_on_column(

    /** The Scan Specifications for the scan */
    1:SpecScan spec

  ) throws (1:Exception e),


  /** The direct method to select cells with result in Key Cells list. */
  KCells  scan_rslt_on_key(

    /** The Scan Specifications for the scan */
    1:SpecScan spec

  ) throws (1:Exception e),


  /** The direct method to select cells with result in Fractons Cells. */
  FCells scan_rslt_on_fraction(

    /** The Scan Specifications for the scan */
    1:SpecScan spec

  ) throws (1:Exception e),


  /** The method to select cells with result set by the request's type of CellsResult. */
  CellsGroup scan_rslt_on(

    /** The Scan Specifications for the scan */
    1:SpecScan spec,

    /** The Type of Cells Result for the response */
    2:CellsResult rslt

  ) throws (

    /** The Base Exception  */
    1:Exception e,

  ),


}

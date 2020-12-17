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
  
  /** Column Key Sequence */
  3: optional KeySeq        col_seq
  
  /** Column Type */
  4: optional ColumnType    col_type


  /** Cell Versions */
  5: optional i32           cell_versions

  /** Cell Time to Live */
  6: optional i32           cell_ttl
  

  /** Block Encoding */
  7: optional EncodingType  blk_encoding

  /** Block Size in Bytes */
  8: optional i32           blk_size

  /** Number of Cells in Block */
  9: optional i32           blk_cells
  

  /** CellStore file Replication */
  10: optional i8            cs_replication

  /** CellStore Size in Bytes */
  11: optional i32          cs_size

  /** Max CellStores in a Range */
  12: optional i8           cs_max


  /** Write Fragment File on ratio reached */
  13: optional i8           log_rollout_ratio

  /** Min. Cointervaling Fragments for Compaction */
  14: optional i8           log_compact_cointervaling

  /** Number of Fragment to Preload */
  15: optional i8           log_fragment_preload


  /** Compact at percent reach */
  16: optional i8           compact_percent
  

  /** Schema's revision/id */
  17: optional i64          revision
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
  DELETE                = 5,

  /** Modify Column Function */
  MODIFY                = 7
}



/** The available logical Comparators, plus extended logic options applied with 'v' for VOLUME */
enum Comp {
  /** [      ]  :   none           (no comparison aplied) */
  NONE = 0x0,

  /** [  =^  ]  :   -pf [prefix]   (starts-with) */ 
  PF   = 0x1,
  
  /** [  >   ]  :   -gt            (greater-than) */
  GT   = 0x2,

  /** [  >=  ]  :   -ge            (greater-equal) */
  GE   = 0x3,

  /** [  =   ]  :   -eq            (equal) */
  EQ   = 0x4,

  /** [  <=  ]  :   -le            (lower-equal) */
  LE   = 0x5,

  /** [  <   ]  :   -lt            (lower-than) */
  LT   = 0x6, 

  /** [  !=  ]  :   -ne            (not-equal) */
  NE   = 0x7,

  /** [  re  ]  :   -re [r,regexp] (regular-expression) */
  RE   = 0x8,
  
  /** [  v>  ]  :   -vgt           (vol greater-than) */
  VGT  = 0x9,

  /** [  v>= ]  :   -vge           (vol greater-equal) */
  VGE  = 0xA,

  /** [  v<= ]  :   -vle           (vol lower-equal) */
  VLE  = 0xB,

  /** [  v<  ]  :   -vlt           (vol lower-than) */
  VLT  = 0xC
}



/** The Schema Matching Pattern for the SpecSchema patterns */
struct SchemaPattern {
  /** Logical comparator to Apply */
  1: Comp   comp

  /** The patern value to match against schema's column name */
  2: string value
}



/** The Specs for Schemas for using with list_columns or compact_columns */
struct SpecSchemas {
  /** The Column IDs */
  1: optional list<i64>             cids

  /** The Column Names */
  2: optional list<string>          names

  /** The Schema's Column Name patterns */
  3: optional list<SchemaPattern>   patterns
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



/** The Fraction Specifications */
struct SpecFraction {
  /** Logical comparator to Apply */
  1: Comp   comp
  /** The binary(bytes) to match against a fraction of a Cell-Key*/
  2: binary f
}

/** The Key Specifications defined as SpecFraction items in a list-container */
typedef list<SpecFraction> SpecKey



/** The Value Specifications, option to use with Extended Logical Comparators  */
struct SpecValue {
  /** Logical comparator to Apply */
  1: Comp   comp

  /** The binary(bytes) to match against the Cell value */
  2: binary v
}



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
  1: optional SpecKey  start

  /** The Key Finish Spec, the finish of cells-interval key match */
  2: optional SpecKey  finish
}

/** The Key Intervals Specifications defined as SpecKeyInterval items in a list-container */
typedef list<SpecKeyInterval> SpecKeyIntervals



/** The Cells Interval Specifications with interval-scope Flags */
struct SpecInterval {
  /** Begin of Ranges evaluation with this Key inclusive */
  1: optional Key               range_begin

  /** End of Ranges evaluation with this Key inclusive */
  2: optional Key               range_end

  /** Offset of Ranges evaluation with this Key inclusive */
  3: optional Key               range_offset

  /** Offset Cell Key of a Scan, select cells from this key inclusive */
  4: optional Key               offset_key

  /** Offset Cell Timestamp of a Scan, select cells after this timestamp  */
  5: optional i64               offset_rev

  /** The Key Intervals */
  6: optional SpecKeyIntervals  key_intervals

  /** The Cell Value Spec, cell-value match */
  7: optional SpecValue         value;
  
  /** The Timestamp Start Spec, the start of cells-interval timestamp match */
  8: optional SpecTimestamp     ts_start
  
  /** The Timestamp Finish Spec, the finish of cells-interval timestamp match */
  9: optional SpecTimestamp    ts_finish
  
  /** The Interval Flags Specification */
  10: optional SpecFlags        flags
}



/** The Column Specifications, the Cells-Intervals(SpecInterval/s) specification for a column */
struct SpecColumn {
  /** The Column ID */
  1: i64                cid

  /** The Cells Interval in a list-container*/
  2: list<SpecInterval> intervals
}



/** The Scan Specifications, the Columns-Intervals(SpecColumn/s) with global-scope Flags */
struct SpecScan {
  /** The Column Intervals(SpecColumn) in a list-container */
  1: list<SpecColumn>    columns

  /** The Global Flags Specification */
  2: optional SpecFlags  flags
}



/** The Cell Flag */
enum Flag {
  /** Unknown/Undefined */
  NONE            = 0,

  /** The Cell is an insert */
  INSERT          = 1,

  /** The Cell is a delete */
  DELETE          = 2,

  /** The Cell is a delete-version */
  DELETE_VERSION  = 3
}



/** The Cell data for using with Update */
struct UCell {
  /** The Cell Flag */
  1: Flag             f
  
  /** The Cell Key */
  2: Key              k

  /** The Cell Timestamp in nanoseconds */
  3: optional i64     ts
  
  /** The Cell Version is in timestamp descending */
  4: optional bool    ts_desc

  /** The Cell Value */
  5: optional binary  v
}

/** The Cells for Update defined as UCell items in a list-container */
typedef list<UCell> UCells

/** The Cells for Update for a Column Id defined as UCells items in a map-container by CID */
typedef map<i64, UCells> UCCells



/** The Cell for results list of scan */
struct Cell {
  /** The Column Name */
  1: string           c

  /** The Cell Key */
  2: Key              k
  
  /** The Cell Timestamp */
  3: i64              ts

  /** The Cell Value */
  4: optional binary  v
}

/** The Cells for results list of scan, defined as Cell items in a list-container */
typedef list<Cell> Cells



/** The Column Cell for results on Columns of scan */
struct CCell {
  /** The Cell Key */
  1: Key              k

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Value */
  3: optional binary  v
}

/** The Column Cells for results on Columns of scan, defined as Cell items in a list-container */
typedef list<CCell> ColCells

/** The Columns Cells for results on Columns of scan, defined as ColCells items in a map-container by Column Name */
typedef map<string, ColCells> CCells



/** The Key Cell for results on Key of scan */
struct KCell {
  /** The Column Name */
  1: string           c

  /** The Cell Timestamp */
  2: i64              ts

  /** The Cell Value */
  3: optional binary  v
}

/** The Key Cells for results on Key of scan */
struct kCells {
  /** The Cell Key */
  1: Key            k

  /** The Key's Cells, defined as KCell items in a list-container */
  2: list<KCell>    cells
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
  3: optional binary  v
}

/** The Fraction Cells for results on Fraction of scan */
struct FCells {

  /** The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes */
  1: map<binary, FCells>   f
  
  /** The current Fraction's Cells, defined as FCell items in a list-container */
  2: optional list<FCell>  cells
}



/** A Grouped Cells result for results of scan, determined by the request's CellsResult enum */
struct CellsGroup {
  /** The Cells in a list, defined as Cell items in a list-container */
  1: optional Cells   cells
  
  /** The Columns Cells in a map-container, defined as ColCells items by Column Name */
  2: optional CCells  ccells
  
  /** The Keys Cells in a list, defined as kCells items in a list-container */
  3: optional KCells  kcells
  
  /** The Fraction Cells in struct FCells */
  4: optional FCells  fcells
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
  1: optional Schemas        schemas
  
  /** Set with result for 'select' query */
  2: optional Cells          cells
  
  /** Set with result for 'compact columns' query */
  3: optional CompactResults compact
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



  /** The direct method to update cells with cell in Update-Columns-Cells, 
    * optionally to work with updater-id. 
    */
  void update(

    /** The Cells to update  */
    1:UCCells cells, 

    /** The Updater ID to use for write */
    2:i64 updater_id = 0

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

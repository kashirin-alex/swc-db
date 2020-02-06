/**
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


/**
 * namespace for target languages
 */

namespace cpp   SWC.Thrift
namespace py    SWC.client.thrift
/*
namespace java  org.swcdb.thrift
namespace perl  SWC.thrift
namespace php   SWC
namespace rb    SWC.thrift
*/


exception Exception {
  1: i32 code
  2: string message
}

/* SCHEMAS */
enum ColumnType {
  UNKNOWN       = 0,
  PLAIN         = 1,
  COUNTER_I64   = 2,
  COUNTER_I32   = 3,
  COUNTER_I16   = 4,
  COUNTER_I8    = 5,
  CELL_DEFINED  = 15
}

enum EncodingType {
  DEFAULT = 0,
  PLAIN   = 1,
  ZLIB    = 2,
  SNAPPY  = 3
}

struct Schema {
  1: optional i64           cid
  2: optional string 	  	  col_name
  3: optional ColumnType    col_type
  
  4: optional i32           cell_versions
  5: optional i32           cell_ttl
  
  6: optional i8            blk_replication
  7: optional EncodingType  blk_encoding
  8: optional i32           blk_size
  9: optional i32           blk_cells
  
  10: optional i32          cs_size
  11: optional i8           cs_max
  12: optional i8           compact_percent
  
  13: optional i64          revision
}
typedef list<Schema> Schemas



/* CELLS */
enum Flag {
  NONE            = 0,
  INSERT          = 1,
  DELETE          = 2,
  DELETE_VERSION  = 3
}

struct Cell {
  1: string           c
  2: list<binary>     k
  3: i64              ts
  4: optional binary  v
}
typedef list<Cell> Cells


struct ColumnMapCell {
  1: list<binary>     k
  2: i64              ts
  3: optional binary  v
}
typedef list<ColumnMapCell> ColumnMapCells
typedef map<string, ColumnMapCells> ColumnsMapCells


struct KeyCell {
  1: string           c
  2: i64              ts
  3: optional binary  v
}
struct KeyCells {
  1: list<binary>     k
  2: list<KeyCell>    cells
}
typedef list<KeyCells> KeysCells


service Service {
  
  Schemas          sql_list_columns(1:string sql) throws (1:Exception e),

  Cells            sql_select_list(1:string sql)  throws (1:Exception e),

  ColumnsMapCells  sql_select_map(1:string sql)   throws (1:Exception e),
  
  KeysCells        sql_select_keys(1:string sql)  throws (1:Exception e),

}

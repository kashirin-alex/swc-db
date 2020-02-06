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

  Cells sql_select_list(1:string sql) throws (1:Exception e),

  ColumnsMapCells sql_select_map(1:string sql) throws (1:Exception e),
  
  KeysCells sql_select_keys(1:string sql) throws (1:Exception e),

}

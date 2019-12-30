
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_types_Column_h
#define swc_lib_db_types_Column_h

#include <string>

namespace SWC { namespace Types { 


enum class Column {
  UNKNOWN       = 0x0,
  PLAIN         = 0x1,
  COUNTER_I64   = 0x2,
  CELL_DEFINED  = 0xf
};

const std::string to_string(Column typ) {
  switch(typ){
    case Column::PLAIN:
      return std::string("PLAIN");
    case Column::COUNTER_I64:
      return std::string("COUNTER_I64");
    case Column::CELL_DEFINED:
      return std::string("CELL_DEFINED");
    default:
      return std::string("UKNOWN");
  }
}

const Column column_from(std::string typ) {

  if(strncasecmp(typ.data(), "PLAIN", typ.length()) == 0 ||
     typ.compare("1") == 0)
    return Column::PLAIN;

  if(strncasecmp(typ.data(), "COUNTER_I64", typ.length()) == 0 ||
    strncasecmp(typ.data(), "COUNTER", typ.length()) == 0 ||
     typ.compare("2") == 0)
    return Column::COUNTER_I64;
  
  return Column::UNKNOWN;
}

}}

#endif // swc_lib_db_types_Column_h

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
  COUNTER_I32   = 0x3,
  COUNTER_I16   = 0x4,
  COUNTER_I8    = 0x5,
  CELL_DEFINED  = 0xf
};

inline const bool is_counter(const Column typ) {
  return typ >= Types::Column::COUNTER_I64 &&
         typ <= Types::Column::COUNTER_I8;
}

inline const std::string to_string(Column typ) {
  switch(typ){
    case Column::PLAIN:
      return std::string("PLAIN");
    case Column::COUNTER_I64:
      return std::string("COUNTER_I64");
    case Column::COUNTER_I32:
      return std::string("COUNTER_I32");
    case Column::COUNTER_I16:
      return std::string("COUNTER_I16");
    case Column::COUNTER_I8:
      return std::string("COUNTER_I8");
    case Column::CELL_DEFINED:
      return std::string("CELL_DEFINED");
    default:
      return std::string("UKNOWN");
  }
}

inline const Column column_type_from(const std::string& typ) {

  if(typ.compare("1") == 0 || 
     strncasecmp(typ.data(), "PLAIN", typ.length()) == 0)
    return Column::PLAIN;

  if(typ.compare("3") == 0 || 
     strncasecmp(typ.data(), "COUNTER_I32", typ.length()) == 0)
    return Column::COUNTER_I32;

  if(typ.compare("4") == 0 || 
     strncasecmp(typ.data(), "COUNTER_I16", typ.length()) == 0)
    return Column::COUNTER_I16;

  if(typ.compare("5") == 0 || 
     strncasecmp(typ.data(), "COUNTER_I8", typ.length()) == 0)
    return Column::COUNTER_I8;

  if(typ.compare("2") == 0 ||
     strncasecmp(typ.data(), "COUNTER_I64", typ.length()) == 0 ||
     strncasecmp(typ.data(), "COUNTER", typ.length()) == 0)
    return Column::COUNTER_I64;
  
  return Column::UNKNOWN;
}


inline const std::string repr_col_type(int typ) {
  return to_string((Column)typ);
}

inline const int from_string_col_type(const std::string& typ) {
  return (int)column_type_from(typ);
}
}}

#endif // swc_lib_db_types_Column_h
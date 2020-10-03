
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/Compat.h"
#include "swcdb/db/Types/Column.h"


namespace SWC { namespace DB { namespace Types {


namespace {
  const char Column_PLAIN[]        = "PLAIN";
  const char Column_COUNTER_I64[]  = "COUNTER_I64";
  const char Column_COUNTER_I32[]  = "COUNTER_I32";
  const char Column_COUNTER_I16[]  = "COUNTER_I16";
  const char Column_COUNTER_I8[]   = "COUNTER_I8";
  const char Column_CELL_DEFINED[] = "CELL_DEFINED";
  const char Column_UNKNOWN[]      = "UNKNOWN";
}


bool is_counter(const Column typ) {
  return typ >= Column::COUNTER_I64 &&
         typ <= Column::COUNTER_I8;
}

std::string to_string(Column typ) {
  switch(typ) {
    case Column::PLAIN:
      return Column_PLAIN;
    case Column::COUNTER_I64:
      return Column_COUNTER_I64;
    case Column::COUNTER_I32:
      return Column_COUNTER_I32;
    case Column::COUNTER_I16:
      return Column_COUNTER_I16;
    case Column::COUNTER_I8:
      return Column_COUNTER_I8;
    case Column::CELL_DEFINED:
      return Column_CELL_DEFINED;
    default:
      return Column_UNKNOWN;
  }
}

Column column_type_from(const std::string& typ) {

  if(typ.compare("1") == 0 || 
     strncasecmp(typ.data(), Column_PLAIN, 5) == 0)
    return Column::PLAIN;

  if(typ.compare("2") == 0 ||
     strncasecmp(typ.data(), Column_COUNTER_I64, typ.length()) == 0)
    return Column::COUNTER_I64;
  
  if(typ.compare("3") == 0 || 
     strncasecmp(typ.data(), Column_COUNTER_I32, 11) == 0)
    return Column::COUNTER_I32;

  if(typ.compare("4") == 0 || 
     strncasecmp(typ.data(), Column_COUNTER_I16, 11) == 0)
    return Column::COUNTER_I16;

  if(typ.compare("5") == 0 || 
     strncasecmp(typ.data(), Column_COUNTER_I8, 10) == 0)
    return Column::COUNTER_I8;

  return Column::UNKNOWN;
}


SWC_SHOULD_INLINE
std::string repr_col_type(int typ) {
  return to_string((Column)typ);
}

SWC_SHOULD_INLINE
int from_string_col_type(const std::string& typ) {
  return (int)column_type_from(typ);
}


}}}

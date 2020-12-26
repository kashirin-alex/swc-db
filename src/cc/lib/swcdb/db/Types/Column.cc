
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


bool is_counter(const Column typ) noexcept {
  return typ >= Column::COUNTER_I64 &&
         typ <= Column::COUNTER_I8;
}

const char* to_string(Column typ) noexcept {
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

Column column_type_from(const std::string& typ) noexcept {
  switch(typ.length()) {
    case 1: {
      switch(*typ.data()) {
        case '1':
          return Column::PLAIN;
        case '2':
          return Column::COUNTER_I64;
        case '3':
          return Column::COUNTER_I32;
        case '4':
          return Column::COUNTER_I16;
        case '5':
          return Column::COUNTER_I8;
        default:
          break;
      }
      break;
    }
    case 5: {
      if(!strncasecmp(typ.data(), Column_PLAIN, 5))
        return Column::PLAIN;
      break;
    }
    case 11: {
      if(!strncasecmp(typ.data(), Column_COUNTER_I64, 11))
        return Column::COUNTER_I64;
      if(!strncasecmp(typ.data(), Column_COUNTER_I32, 11))
        return Column::COUNTER_I32;
      if(!strncasecmp(typ.data(), Column_COUNTER_I16, 11))
        return Column::COUNTER_I16;
      break;
    }
    case 10: {
      if(!strncasecmp(typ.data(), Column_COUNTER_I8, 10))
        return Column::COUNTER_I8;
      break;
    }
    default:
      break;
  }
  return Column::UNKNOWN;
}


SWC_SHOULD_INLINE
std::string repr_col_type(int typ) {
  return to_string((Column)typ);
}

SWC_SHOULD_INLINE
int from_string_col_type(const std::string& typ) noexcept {
  return (int)column_type_from(typ);
}


}}}


/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_Column_h
#define swc_db_types_Column_h

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

bool is_counter(const Column typ);

std::string to_string(Column typ);

Column column_type_from(const std::string& typ);


std::string repr_col_type(int typ);

int from_string_col_type(const std::string& typ);

}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/Column.cc"
#endif 

#endif // swc_db_types_Column_h
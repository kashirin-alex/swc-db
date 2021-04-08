/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_Column_h
#define swcdb_db_types_Column_h

#include <string>

namespace SWC { namespace DB {


//! The SWC-DB Types C++ namespace 'SWC::DB::Types'
namespace Types {


enum class Column : uint8_t {
  UNKNOWN       = 0x0,
  PLAIN         = 0x1,
  COUNTER_I64   = 0x2,
  COUNTER_I32   = 0x3,
  COUNTER_I16   = 0x4,
  COUNTER_I8    = 0x5,
  SERIAL        = 0x6,
  CELL_DEFINED  = 0xf
};

bool is_counter(const Column typ) noexcept;

const char* to_string(Column typ) noexcept;

Column column_type_from(const std::string& typ) noexcept;


std::string repr_col_type(int typ);

int from_string_col_type(const std::string& typ) noexcept;

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/Column.cc"
#endif

#endif // swcdb_db_types_Column_h

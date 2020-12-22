
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_Range_h
#define swcdb_db_types_Range_h

#include <string>

namespace SWC { namespace DB { namespace Types { 


enum class Range : uint8_t {
  MASTER  = 1,
  META    = 2,
  DATA    = 3
};

const char* to_string(Range typ) noexcept;

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/Range.cc"
#endif 

#endif // swcdb_db_types_Range_h
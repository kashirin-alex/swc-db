
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_types_Range_h
#define swc_db_types_Range_h

#include <string>

namespace SWC { namespace DB { namespace Types { 

enum Range {
  MASTER  = 1,
  META    = 2,
  DATA    = 3
};

std::string to_string(Range typ);

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/Range.cc"
#endif 

#endif // swc_db_types_Range_h
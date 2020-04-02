
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_Range_h
#define swc_db_types_Range_h

#include <string>

namespace SWC { namespace Types { 

enum Range {
  MASTER  = 1,
  META    = 2,
  DATA    = 3
};

std::string to_string(Range typ);

}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/Range.cc"
#endif 

#endif // swc_db_types_Range_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_MngrState_h
#define swcdb_db_types_MngrState_h

#include <string>

namespace SWC { namespace DB { namespace Types { 


enum class MngrState : uint8_t {
  NOTSET    = 0,
  OFF       = 1,
  STANDBY   = 2,
  WANT      = 3,
  NOMINATED = 4,
  ACTIVE    = 5
};


const char* to_string(MngrState state) noexcept;


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrState.cc"
#endif 

#endif // swcdb_db_types_MngrState_h

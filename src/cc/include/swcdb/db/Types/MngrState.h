
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_types_MngrState_h
#define swc_db_types_MngrState_h

#include <string>

namespace SWC { namespace Types { 


enum MngrState {
  NOTSET    = 0,
  OFF       = 1,
  STANDBY   = 2,
  WANT      = 3,
  NOMINATED = 4,
  ACTIVE    = 5
};


std::string to_string(MngrState state);


}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrState.cc"
#endif 

#endif // swc_db_types_MngrState_h

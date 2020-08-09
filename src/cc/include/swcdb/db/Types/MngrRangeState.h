
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_types_MngrRangeState_h
#define swc_db_types_MngrRangeState_h

#include <string>

namespace SWC { namespace Types {  


namespace MngrRange {

  enum State {
    NOTSET    = 0,
    DELETED   = 1,
    ASSIGNED  = 2,
    CREATED   = 3,
    QUEUED    = 4
  };

}


std::string to_string(MngrRange::State state);


}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrRangeState.cc"
#endif 

#endif // swc_db_types_MngrRangeState_h

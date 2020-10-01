
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_MngrRangeState_h
#define swcdb_db_types_MngrRangeState_h

#include <string>

namespace SWC { namespace DB { namespace Types { 


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


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrRangeState.cc"
#endif 

#endif // swcdb_db_types_MngrRangeState_h

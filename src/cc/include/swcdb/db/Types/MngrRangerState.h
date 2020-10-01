
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_MngrRangerState_h
#define swcdb_db_types_MngrRangerState_h

#include <string>

namespace SWC { namespace DB { namespace Types { 


namespace MngrRanger {

  enum State {
    NONE            = 0,
    AWAIT           = 1,
    ACK             = 2,
    REMOVED         = 3,
    MARKED_OFFLINE  = 4
  };

}


std::string to_string(MngrRanger::State state);


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrRangerState.cc"
#endif 

#endif // swcdb_db_types_MngrRangerState_h


/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrRangerState.h"


namespace SWC { namespace DB { namespace Types {


std::string to_string(MngrRanger::State state) {
  switch(state) {

    case MngrRanger::State::NONE:
      return std::string("NONE");

    case MngrRanger::State::AWAIT:
      return std::string("AWAIT");

    case MngrRanger::State::ACK:
      return std::string("ACK");

    case MngrRanger::State::REMOVED:
      return std::string("REMOVED");

    case MngrRanger::State::MARKED_OFFLINE:
      return std::string("MARKED_OFFLINE");

    default:
      return std::string("uknown");
  }
}


}}}

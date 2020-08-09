
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrRangerState.h"


namespace SWC { namespace Types { 


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

    default:
      return std::string("uknown");
  }
}


}}

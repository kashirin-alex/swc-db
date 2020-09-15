
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrState.h"


namespace SWC { namespace Types {


std::string to_string(MngrState state) {
  switch(state) {

    case MngrState::NOTSET:
      return std::string("NOTSET");

    case MngrState::OFF:
      return std::string("OFF");

    case MngrState::STANDBY:
      return std::string("STANDBY");

    case MngrState::WANT:
      return std::string("WANT");

    case MngrState::NOMINATED:
      return std::string("NOMINATED");

    case MngrState::ACTIVE:
      return std::string("ACTIVE");

    default:
      return std::string("uknown");
  }
}


}}

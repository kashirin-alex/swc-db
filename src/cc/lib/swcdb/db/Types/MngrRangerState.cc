
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrRangerState.h"


namespace SWC { namespace DB { namespace Types {

namespace {
  const char MngrRanger_State_NONE[]            = "NONE";
  const char MngrRanger_State_OK[]              = "OK";
  const char MngrRanger_State_AWAIT[]           = "AWAIT";
  const char MngrRanger_State_ACK[]             = "ACK";
  const char MngrRanger_State_REMOVED[]         = "REMOVED";
  const char MngrRanger_State_MARKED_OFFLINE[]  = "MARKED_OFFLINE";
  const char MngrRanger_State_UNKNOWN[]         = "UNKNOWN";
}

std::string to_string(MngrRanger::State state) {
  switch(state) {

    case MngrRanger::State::NONE:
      return MngrRanger_State_NONE;

    case MngrRanger::State::AWAIT:
      return MngrRanger_State_AWAIT;

    case MngrRanger::State::ACK:
      return MngrRanger_State_ACK;

    case MngrRanger::State::REMOVED:
      return MngrRanger_State_REMOVED;

    case MngrRanger::State::MARKED_OFFLINE:
      return MngrRanger_State_MARKED_OFFLINE;

    default:
      return MngrRanger_State_UNKNOWN;
  }
}


}}}

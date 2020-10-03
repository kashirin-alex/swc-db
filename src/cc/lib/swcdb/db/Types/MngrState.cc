
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrState.h"


namespace SWC { namespace DB { namespace Types {


namespace {
  const char MngrState_NOTSET[]     = "NOTSET";
  const char MngrState_OFF[]        = "OFF";
  const char MngrState_STANDBY[]    = "STANDBY";
  const char MngrState_WANT[]       = "WANT";
  const char MngrState_NOMINATED[]  = "NOMINATED";
  const char MngrState_ACTIVE[]     = "ACTIVE";
  const char MngrState_UNKNOWN[]    = "UNKNOWN";
}


std::string to_string(MngrState state) {
  switch(state) {

    case MngrState::NOTSET:
      return MngrState_NOTSET;

    case MngrState::OFF:
      return MngrState_OFF;

    case MngrState::STANDBY:
      return MngrState_STANDBY;

    case MngrState::WANT:
      return MngrState_WANT;

    case MngrState::NOMINATED:
      return MngrState_NOMINATED;

    case MngrState::ACTIVE:
      return MngrState_ACTIVE;

    default:
      return MngrState_UNKNOWN;
  }
}


}}}

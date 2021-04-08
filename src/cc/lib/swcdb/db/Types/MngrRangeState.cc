/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrRangeState.h"


namespace SWC { namespace DB { namespace Types {


namespace {
  const char MngrRange_State_NOTSET[]    = "NOTSET";
  const char MngrRange_State_DELETED[]   = "DELETED";
  const char MngrRange_State_ASSIGNED[]  = "ASSIGNED";
  const char MngrRange_State_CREATED[]   = "CREATED";
  const char MngrRange_State_QUEUED[]    = "QUEUED";
  const char MngrRange_State_MERGE[]     = "MERGE";
  const char MngrRange_State_UNKNOWN[]   = "UNKNOWN";
}


const char* to_string(MngrRange::State state) noexcept {
  switch(state) {

    case MngrRange::State::NOTSET:
      return MngrRange_State_NOTSET;

    case MngrRange::State::DELETED:
      return MngrRange_State_DELETED;

    case MngrRange::State::ASSIGNED:
      return MngrRange_State_ASSIGNED;

    case MngrRange::State::CREATED:
      return MngrRange_State_CREATED;

    case MngrRange::State::QUEUED:
      return MngrRange_State_QUEUED;

    case MngrRange::State::MERGE:
      return MngrRange_State_MERGE;

    default:
      return MngrRange_State_UNKNOWN;
  }
}


}}}

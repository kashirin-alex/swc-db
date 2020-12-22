
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrRangerState.h"


namespace SWC { namespace DB { namespace Types { namespace MngrRangerState {

namespace {
  const char STR_NONE[]              = "NONE";
  const char STR_OK[]                = "OK";
  const char STR_AWAIT[]             = "AWAIT";
  const char STR_ACK[]               = "ACK";
  const char STR_REMOVED[]           = "REMOVED";
  const char STR_MARKED_OFFLINE[]    = "MARKED_OFFLINE";
  const char STR_SHUTTINGDOWN[]      = "SHUTTINGDOWN";
  const char STR_ACK_SHUTTINGDOWN[]  = "ACK&SHUTTINGDOWN";
  const char STR_UNKNOWN[]           = "UNKNOWN";
}

const char* to_string(uint8_t state) noexcept {
  switch(state) {

    case NONE:
      return STR_NONE;

    case AWAIT:
      return STR_AWAIT;

    case ACK:
      return STR_ACK;

    case REMOVED:
      return STR_REMOVED;

    case MARKED_OFFLINE:
      return STR_MARKED_OFFLINE;

    case SHUTTINGDOWN:
      return STR_SHUTTINGDOWN;

    default:
      if(state & ACK && state & SHUTTINGDOWN)
        return STR_ACK_SHUTTINGDOWN;
      return STR_UNKNOWN;
  }
}


}}}}

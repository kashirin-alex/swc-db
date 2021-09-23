/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_MngrRangerState_h
#define swcdb_db_types_MngrRangerState_h

#include "swcdb/core/Compat.h"

namespace SWC { namespace DB { namespace Types {


namespace MngrRangerState {

const uint8_t NONE            = 0x01;
const uint8_t AWAIT           = 0x02;
const uint8_t ACK             = 0x04;
const uint8_t REMOVED         = 0x08;
const uint8_t MARKED_OFFLINE  = 0x10;
const uint8_t SHUTTINGDOWN    = 0x20;


const char* SWC_CONST_FUNC to_string(uint8_t state) noexcept;

}




}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrRangerState.cc"
#endif

#endif // swcdb_db_types_MngrRangerState_h

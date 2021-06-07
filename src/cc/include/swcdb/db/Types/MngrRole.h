/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_MngrRole_h
#define swcdb_db_types_MngrRole_h

#include "swcdb/core/Compat.h"

namespace SWC { namespace DB { namespace Types { namespace MngrRole {

const uint8_t NONE       = 0x00;
const uint8_t COLUMNS    = 0x01;
const uint8_t SCHEMAS    = 0x02;
const uint8_t RANGERS    = 0x04;
const uint8_t NO_COLUMNS = 0x08;
const uint8_t ALL = COLUMNS | SCHEMAS | RANGERS;

std::string to_string(uint8_t role);

}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrRole.cc"
#endif

#endif // swcdb_db_types_MngrRole_h

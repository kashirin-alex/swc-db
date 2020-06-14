
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_MngrRole_h
#define swc_db_types_MngrRole_h

#include "swcdb/core/Compat.h"

namespace SWC { namespace Types { namespace MngrRole {

const uint8_t NONE       = 0x00;
const uint8_t COLUMNS    = 0x01;
const uint8_t SCHEMAS    = 0x02;
const uint8_t RANGERS    = 0x04;
const uint8_t NO_COLUMNS = 0x08;
const uint8_t ALL = COLUMNS | SCHEMAS | RANGERS;

std::string to_string(uint8_t role);

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MngrRole.cc"
#endif 

#endif // swc_db_types_MngrRole_h
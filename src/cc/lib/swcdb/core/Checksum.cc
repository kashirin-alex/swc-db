/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/core/Logger.h"

namespace SWC {



void checksum_i32_chk_err(uint32_t original, uint32_t computed) {
  SWC_LOGF(LOG_ERROR, "checksum_i32_chk, original(%u) != computed(%u)", 
            original, computed);
}



} // namespace SWC

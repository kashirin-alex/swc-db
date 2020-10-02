/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/core/Logger.h"

namespace SWC { namespace Core {



void checksum_i32_chk_err(uint32_t original, uint32_t computed) {
  SWC_LOGF(LOG_ERROR, "checksum_i32_chk, original(%u) != computed(%u)", 
            original, computed);
}



}} // namespace SWC::Core

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Checksum.h"
#include "swcdb/core/Logger.h"

namespace SWC { namespace Core {



bool checksum_i32_log_chk(uint32_t checksum, const uint8_t* base,
                          uint32_t len) {
  uint32_t computed = checksum32(base, len);
  if(checksum == computed)
    return true;
  SWC_LOGF(LOG_ERROR,
    "checksum32, original(%u) != computed(%u)",
    checksum, computed);
  return false;
}



}} // namespace SWC::Core

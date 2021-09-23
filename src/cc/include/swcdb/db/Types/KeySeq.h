/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_KeySeq_h
#define swcdb_db_types_KeySeq_h

#include "swcdb/core/Compat.h"

namespace SWC { namespace DB { namespace Types {

enum class KeySeq : uint8_t {
  UNKNOWN     = 0,
  LEXIC       = 1,
  VOLUME      = 2,
  FC_LEXIC    = 3,
  FC_VOLUME   = 4
};

bool SWC_CONST_FUNC is_fc(KeySeq typ) noexcept;

const char* SWC_CONST_FUNC to_string(KeySeq typ) noexcept;

KeySeq SWC_PURE_FUNC range_seq_from(const std::string& typ) noexcept;


SWC_CAN_INLINE
std::string repr_range_seq(int typ) {
  return to_string(KeySeq(typ));
}

SWC_CAN_INLINE
int from_string_range_seq(const std::string& typ) noexcept {
  return int(range_seq_from(typ));
}


}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/KeySeq.cc"
#endif

#endif // swcdb_db_types_KeySeq_h


/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_KeySeq_h
#define swcdb_db_types_KeySeq_h

#include <string>

namespace SWC { namespace DB { namespace Types { 

enum class KeySeq : uint8_t {
  UNKNOWN     = 0,
  LEXIC       = 1,
  VOLUME      = 2, 
  FC_LEXIC    = 3,
  FC_VOLUME   = 4
};

bool is_fc(KeySeq typ) noexcept;

const char* to_string(KeySeq typ) noexcept;

KeySeq range_seq_from(const std::string& typ) noexcept;

std::string repr_range_seq(int typ);

int from_string_range_seq(const std::string& typ) noexcept;

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/KeySeq.cc"
#endif 

#endif // swcdb_db_types_KeySeq_h
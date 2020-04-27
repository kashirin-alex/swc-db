
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_KeySeq_h
#define swc_db_types_KeySeq_h

#include <string>

namespace SWC { namespace Types { 

enum KeySeq {
  UNKNOWN     = 0,
  LEXIC       = 1,
  VOLUME      = 2, 
  FC_LEXIC    = 3,
  FC_VOLUME   = 4
};

std::string to_string(KeySeq typ);

KeySeq range_seq_from(const std::string& typ);

std::string repr_range_seq(int typ);

int from_string_range_seq(const std::string& typ);

}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/KeySeq.cc"
#endif 

#endif // swc_db_types_KeySeq_h
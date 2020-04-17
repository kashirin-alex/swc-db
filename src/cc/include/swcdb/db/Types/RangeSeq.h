
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_RangeSeq_h
#define swc_db_types_RangeSeq_h

#include <string>

namespace SWC { namespace Types { 

enum RangeSeq {
  UNKNOWN             = 0,
  BITWISE             = 1,
  BITWISE_VOL         = 2, 
  BITWISE_FCOUNT      = 3,
  BITWISE_VOL_FCOUNT  = 4
};

std::string to_string(RangeSeq typ);

RangeSeq range_seq_from(const std::string& typ);

std::string repr_range_seq(int typ);

int from_string_range_seq(const std::string& typ);

}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/RangeSeq.cc"
#endif 

#endif // swc_db_types_RangeSeq_h

/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/RangeSeq.h"
#include <cstring>


namespace SWC { namespace Types { 


std::string to_string(RangeSeq typ) {
  switch(typ){
    case RangeSeq::BITWISE:
      return std::string("BITWISE");
    case RangeSeq::BITWISE_VOL:
      return std::string("BITWISE_VOL");
    case RangeSeq::BITWISE_FCOUNT:
      return std::string("BITWISE_FCOUNT");
    case RangeSeq::BITWISE_VOL_FCOUNT:
      return std::string("BITWISE_VOL_FCOUNT");
    default:
      return std::string("uknown");
  }
}

RangeSeq range_seq_from(const std::string& typ) {
  if(typ.compare("1") == 0 || 
     strncasecmp(typ.data(), "BITWISE", typ.length()) == 0)
    return RangeSeq::BITWISE;

  if(typ.compare("2") == 0 || 
     strncasecmp(typ.data(), "BITWISE_VOL", typ.length()) == 0)
    return RangeSeq::BITWISE_VOL;

  if(typ.compare("3") == 0 || 
     strncasecmp(typ.data(), "BITWISE_FCOUNT", typ.length()) == 0)
    return RangeSeq::BITWISE_FCOUNT;

  if(typ.compare("4") == 0 || 
     strncasecmp(typ.data(), "BITWISE_VOL_FCOUNT", typ.length()) == 0)
    return RangeSeq::BITWISE_VOL_FCOUNT;

  return RangeSeq::UNKNOWN;
}


std::string repr_range_seq(int typ) {
  return to_string((RangeSeq)typ);
}

int from_string_range_seq(const std::string& typ) {
  return (int)range_seq_from(typ);
}

}}

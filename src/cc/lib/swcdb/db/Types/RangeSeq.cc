
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/RangeSeq.h"


namespace SWC { namespace Types { 


const std::string to_string(RangeSeq typ) {
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


}}

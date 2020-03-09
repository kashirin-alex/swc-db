
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_types_RangeSeq_h
#define swc_db_types_RangeSeq_h


namespace SWC { namespace Types { 

enum RangeSeq{
  BITWISE             = 1,
  BITWISE_VOL         = 2, 
  BITWISE_FCOUNT      = 3,
  BITWISE_VOL_FCOUNT  = 4
};

inline const std::string to_string(RangeSeq typ) {
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

#endif // swc_db_types_RangeSeq_h
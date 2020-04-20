
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/KeySeq.h"
#include <cstring>


namespace SWC { namespace Types { 


std::string to_string(KeySeq typ) {
  switch(typ){
    case KeySeq::BITWISE:
      return std::string("BITWISE");
    case KeySeq::VOLUME:
      return std::string("VOLUME");
    case KeySeq::BITWISE_FCOUNT:
      return std::string("BITWISE_FCOUNT");
    case KeySeq::VOLUME_FCOUNT:
      return std::string("VOLUME_FCOUNT");
    default:
      return std::string("uknown");
  }
}

KeySeq range_seq_from(const std::string& typ) {
  if(typ.compare("1") == 0 || 
      (typ.length() == 7 && 
       strncasecmp(typ.data(), "BITWISE", 7) == 0))
      return KeySeq::BITWISE;
      
  if(typ.compare("2") == 0 || 
    (typ.length() == 6 && 
     strncasecmp(typ.data(), "VOLUME", 6) == 0))
  return KeySeq::VOLUME;
    
  if(typ.compare("3") == 0 || 
      (typ.length() == 14 && 
       strncasecmp(typ.data(), "BITWISE_FCOUNT", 14) == 0))
    return KeySeq::BITWISE_FCOUNT;

  if(typ.compare("4") == 0 || 
      (typ.length() == 13 && 
       strncasecmp(typ.data(), "VOLUME_FCOUNT", 13) == 0))
    return KeySeq::VOLUME_FCOUNT;

  return KeySeq::UNKNOWN;
}


std::string repr_range_seq(int typ) {
  return to_string((KeySeq)typ);
}

int from_string_range_seq(const std::string& typ) {
  return (int)range_seq_from(typ);
}

}}

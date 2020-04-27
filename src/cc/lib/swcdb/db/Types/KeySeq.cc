
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/KeySeq.h"
#include <cstring>


namespace SWC { namespace Types { 


std::string to_string(KeySeq typ) {
  switch(typ){
    case KeySeq::LEXIC:
      return std::string("LEXIC");
    case KeySeq::VOLUME:
      return std::string("VOLUME");
    case KeySeq::FC_LEXIC:
      return std::string("FC_LEXIC");
    case KeySeq::FC_VOLUME:
      return std::string("FC_VOLUME");
    default:
      return std::string("uknown");
  }
}

KeySeq range_seq_from(const std::string& typ) {
  if(typ.compare("1") == 0 || 
      (typ.length() == 5 && 
       strncasecmp(typ.data(), "LEXIC", 5) == 0))
      return KeySeq::LEXIC;
      
  if(typ.compare("2") == 0 || 
    (typ.length() == 6 && 
     strncasecmp(typ.data(), "VOLUME", 6) == 0))
  return KeySeq::VOLUME;
    
  if(typ.compare("3") == 0 || 
      (typ.length() == 8 && 
       strncasecmp(typ.data(), "FC_LEXIC", 8) == 0))
    return KeySeq::FC_LEXIC;

  if(typ.compare("4") == 0 || 
      (typ.length() == 9 && 
       strncasecmp(typ.data(), "FC_VOLUME", 9) == 0))
    return KeySeq::FC_VOLUME;

  return KeySeq::UNKNOWN;
}


std::string repr_range_seq(int typ) {
  return to_string((KeySeq)typ);
}

int from_string_range_seq(const std::string& typ) {
  return (int)range_seq_from(typ);
}

}}

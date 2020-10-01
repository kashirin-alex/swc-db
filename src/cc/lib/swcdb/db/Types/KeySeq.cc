
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/Compat.h"
#include "swcdb/db/Types/KeySeq.h"


namespace SWC { namespace DB { namespace Types {


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


SWC_SHOULD_INLINE
std::string repr_range_seq(int typ) {
  return to_string((KeySeq)typ);
}

SWC_SHOULD_INLINE
int from_string_range_seq(const std::string& typ) {
  return (int)range_seq_from(typ);
}

}}}

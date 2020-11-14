
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/Compat.h"
#include "swcdb/db/Types/KeySeq.h"


namespace SWC { namespace DB { namespace Types {


namespace {
  const char KeySeq_LEXIC[]      = "LEXIC";
  const char KeySeq_VOLUME[]     = "VOLUME";
  const char KeySeq_FC_LEXIC[]   = "FC_LEXIC";
  const char KeySeq_FC_VOLUME[]  = "FC_VOLUME";
  const char KeySeq_UNKNOWN[]    = "UNKNOWN";
}


std::string to_string(KeySeq typ) {
  switch(typ) {
    case KeySeq::LEXIC:
      return KeySeq_LEXIC;
    case KeySeq::VOLUME:
      return KeySeq_VOLUME;
    case KeySeq::FC_LEXIC:
      return KeySeq_FC_LEXIC;
    case KeySeq::FC_VOLUME:
      return KeySeq_FC_VOLUME;
    default:
      return KeySeq_UNKNOWN;
  }
}

KeySeq range_seq_from(const std::string& typ) {
  if(!typ.compare("1") || 
      (typ.length() == 5 && !strncasecmp(typ.data(), KeySeq_LEXIC, 5)))
      return KeySeq::LEXIC;
      
  if(!typ.compare("2") || 
    (typ.length() == 6 && !strncasecmp(typ.data(), KeySeq_VOLUME, 6)))
  return KeySeq::VOLUME;
    
  if(!typ.compare("3") || 
      (typ.length() == 8 && !strncasecmp(typ.data(), KeySeq_FC_LEXIC, 8)))
    return KeySeq::FC_LEXIC;

  if(!typ.compare("4") || 
      (typ.length() == 9 && !strncasecmp(typ.data(), KeySeq_FC_VOLUME, 9)))
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

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

bool is_fc(KeySeq typ) noexcept {
  switch(typ) {
    case KeySeq::FC_LEXIC:
    case KeySeq::FC_VOLUME:
      return true;
    default:
      return false;
  }
}


const char* to_string(KeySeq typ) noexcept {
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

KeySeq range_seq_from(const std::string& typ) noexcept {
  switch(typ.length()) {
    case 1: {
      switch(*typ.data()) {
        case '1':
          return KeySeq::LEXIC;
        case '2':
          return KeySeq::VOLUME;
        case '3':
          return KeySeq::FC_LEXIC;
        case '4':
          return KeySeq::FC_VOLUME;
        default:
          break;
      }
      break;
    }
    case 5: {
      if(Condition::str_case_eq(typ.data(), KeySeq_LEXIC, 5))
        return KeySeq::LEXIC;
      break;
    }
    case 6: {
      if(Condition::str_case_eq(typ.data(), KeySeq_VOLUME, 6))
        return KeySeq::VOLUME;
      break;
    }
    case 8: {
      if(Condition::str_case_eq(typ.data(), KeySeq_FC_LEXIC, 8))
        return KeySeq::FC_LEXIC;
      break;
    }
    case 9: {
      if(Condition::str_case_eq(typ.data(), KeySeq_FC_VOLUME, 9))
        return KeySeq::FC_VOLUME;
      break;
    }
    default:
      break;
  }
  return KeySeq::UNKNOWN;
}


SWC_SHOULD_INLINE
std::string repr_range_seq(int typ) {
  return to_string(KeySeq(typ));
}

SWC_SHOULD_INLINE
int from_string_range_seq(const std::string& typ) noexcept {
  return int(range_seq_from(typ));
}

}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/Comparators.h"


namespace SWC { namespace Condition {



Comp from(const char** buf, uint32_t* remainp, uint8_t extended)  noexcept {
  Comp comp = Comp::NONE;

  if(*remainp > 7) {
    if(str_case_eq(*buf, "posubset", 8))
      comp = Comp::POSBS;
    else if(str_case_eq(*buf, "posupset", 8))
      comp = Comp::POSPS;
    else if(str_case_eq(*buf, "fosubset", 8))
      comp = Comp::FOSBS;
    else if(str_case_eq(*buf, "fosupset", 8))
      comp = Comp::FOSPS;

    if(comp != Comp::NONE) {
      *buf += 8;
      *remainp -= 8;
      return comp;
    }
  }

  if(*remainp > 5) {
    if(str_case_eq(*buf, "subset", 6))
      comp = Comp::SBS;
    else if(str_case_eq(*buf, "supset", 6))
      comp = Comp::SPS;

    if(comp != Comp::NONE) {
      *buf += 6;
      *remainp -= 6;
      return comp;
    }
  }

  if(*remainp > 4) {
    if(str_case_eq(*buf, "posbs", 5))
      comp = Comp::POSBS;
    else if(str_case_eq(*buf, "posps", 5))
      comp = Comp::POSPS;
    else if(str_case_eq(*buf, "fosbs", 5))
      comp = Comp::FOSBS;
    else if(str_case_eq(*buf, "fosps", 5))
      comp = Comp::FOSPS;

    if(comp != Comp::NONE) {
      *buf += 5;
      *remainp -= 5;
      return comp;
    }
  }

  if(*remainp > 2) {
    if(str_case_eq(*buf, "sbs", 3)) {
      comp = Comp::SBS;
    } else if(str_case_eq(*buf, "sps", 3)) {
      comp = Comp::SPS;
    } else if(extended) {
      if (extended & COMP_EXTENDED_VALUE) {
        if(str_case_eq(*buf, COMP_VGE, 3) ||
          str_case_eq(*buf, "vge", 3))
          comp = Comp::VGE;
        else if(str_case_eq(*buf, COMP_VLE, 3) ||
                str_case_eq(*buf, "vle", 3))
          comp = Comp::VLE;
        else if(str_case_eq(*buf, "vgt", 3))
          comp = Comp::VGT;
        else if(str_case_eq(*buf, "vlt", 3))
          comp = Comp::VLT;
      }
      if(extended & COMP_EXTENDED_KEY) {
        if(str_case_eq(*buf, "fip", 3))
          comp = Comp::FIP;
      }
    }
    if(comp != Comp::NONE) {
      *buf += 3;
      *remainp -= 3;
      return comp;
    }
  }

  if(*remainp > 1) {
    if(str_case_eq(*buf, COMP_PF, 2) ||
       str_case_eq(*buf, "pf", 2))
      comp = Comp::PF;
    else if(str_case_eq(*buf, COMP_GE, 2) ||
            str_case_eq(*buf, "ge", 2))
      comp = Comp::GE;
    else if(str_case_eq(*buf, COMP_LE, 2) ||
            str_case_eq(*buf, "le", 2))
      comp = Comp::LE;
    else if(str_case_eq(*buf, COMP_NE, 2) ||
            str_case_eq(*buf, "ne", 2))
      comp = Comp::NE;
    else if(str_case_eq(*buf, COMP_RE, 2))
      comp = Comp::RE;
    else if(str_case_eq(*buf, COMP_EQ, 2) ||
            str_case_eq(*buf, "eq", 2))
      comp = Comp::EQ;
    else if(str_case_eq(*buf, "gt", 2))
      comp = Comp::GT;
    else if(str_case_eq(*buf, "lt", 2))
      comp = Comp::LT;
    else if(str_case_eq(*buf, COMP_SBS, 2))
      comp = Comp::SBS;
    else if(str_case_eq(*buf, COMP_SPS, 2))
      comp = Comp::SPS;
    else if(str_case_eq(*buf, COMP_POSBS, 2))
      comp = Comp::POSBS;
    else if(str_case_eq(*buf, COMP_POSPS, 2))
      comp = Comp::POSPS;
    else if(str_case_eq(*buf, COMP_FOSBS, 2))
      comp = Comp::FOSBS;
    else if(str_case_eq(*buf, COMP_FOSPS, 2)) {
      comp = Comp::FOSPS;
    } else if(extended) {
      if(extended & COMP_EXTENDED_VALUE) {
        if(str_case_eq(*buf, COMP_VGT, 2))
          comp = Comp::VGT;
        else if(str_case_eq(*buf, COMP_VLT, 2))
          comp = Comp::VLT;
      }
      if(extended & COMP_EXTENDED_KEY) {
        if(str_case_eq(*buf, COMP_FIP, 2))
          comp = Comp::FIP;
        else if(str_case_eq(*buf, "fi", 2))
          comp = Comp::FI;
      }
    }

    if(comp != Comp::NONE) {
      *buf += 2;
      *remainp -= 2;
      return comp;
    }
  }

  if(*remainp > 0) {
    if(**buf == '>')
      comp = Comp::GT;
    else if(**buf == '<')
      comp = Comp::LT;
    else if(**buf == '=')
      comp = Comp::EQ;
    else if(extended & COMP_EXTENDED_KEY && **buf == ':')
      comp = Comp::FI;
    else if(**buf == 'r' || **buf == 'R')
      comp = Comp::RE;

    if(comp != Comp::NONE) {
      *buf += 1;
      *remainp -= 1;
      return comp;
    }
  }

  return comp;
}


const char* to_string(Comp comp, uint8_t extended) noexcept {

  if(extended) {
    if(extended & COMP_EXTENDED_VALUE) switch (comp) {
      case Comp::VGT:
        return COMP_VGT;
      case Comp::VGE:
        return COMP_VGE;
      case Comp::VLE:
        return COMP_VLE;
      case Comp::VLT:
        return COMP_VLT;
      default:
        break;
    }
    if(extended & COMP_EXTENDED_KEY) switch (comp) {
      case Comp::FIP:
        return COMP_FIP;
      case Comp::FI:
        return COMP_FI;
      default:
        break; 
    } 
  }

  switch (comp) {
    case Comp::EQ:
      return COMP_EQ;
    case Comp::PF:
      return COMP_PF;
    case Comp::GT:
      return COMP_GT;
    case Comp::GE:
      return COMP_GE;
    case Comp::LE:
      return COMP_LE;
    case Comp::LT:
      return COMP_LT;
    case Comp::NE:
      return COMP_NE;
    case Comp::RE:
      return COMP_RE;
    case Comp::SBS:
      return COMP_SBS;
    case Comp::SPS:
      return COMP_SPS;
    case Comp::POSBS:
      return COMP_POSBS;
    case Comp::POSPS:
      return COMP_POSPS;
    case Comp::FOSBS:
      return COMP_FOSBS;
    case Comp::FOSPS:
      return COMP_FOSPS;
    default:
      return COMP_NONE;
  }
}




bool re(const re2::RE2& regex, const re2::StringPiece& value) {
  return re2::RE2::PartialMatch(value, regex);
}



} } // namespace SWC::Condition

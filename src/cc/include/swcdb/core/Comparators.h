/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_core_Comparators_h
#define swcdb_core_Comparators_h

#include <memory>
#include <cstring>
#include <re2/re2.h>


# define SWC_CAN_INLINE  \
  __attribute__((__always_inline__, __artificial__)) \
  extern inline

# define SWC_NOINLINE  \
  __attribute__((__noinline__)) \
  static 


namespace SWC { 

SWC_NOINLINE
int
_memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) {
  for(; count; --count, ++s1, ++s2)
    if(*s1 != *s2) 
      return *s1 < *s2 ? -1 : 1;
  return 0;
}
  
SWC_CAN_INLINE   
int
memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) {
  return _memcomp(s1, s2, count);
}

namespace Condition {


enum Comp {
  NONE = 0x0,   // [      ]  :   none           (no comparison aplied)
  PF   = 0x1,   // [  =^  ]  :   -pf [prefix]   (starts-with)
  GT   = 0x2,   // [  >   ]  :   -gt            (greater-than)
  GE   = 0x3,   // [  >=  ]  :   -ge            (greater-equal)
  EQ   = 0x4,   // [  =   ]  :   -eq            (equal)
  LE   = 0x5,   // [  <=  ]  :   -le            (lower-equal)
  LT   = 0x6,   // [  <   ]  :   -lt            (lower-than)
  NE   = 0x7,   // [  !=  ]  :   -ne            (not-equal)
  RE   = 0x8,   // [  re  ]  :   -re [r,regexp] (regular-expression)
  
  // extended logic options: ge,le,gt,lt are LEXIC and with 'V' VOLUME
  VGT  = 0x9,   // [  v>  ]  :   -vgt           (vol greater-than)
  VGE  = 0xA,   // [  v>= ]  :   -vge           (vol greater-equal)
  VLE  = 0xB,   // [  v<= ]  :   -vle           (vol lower-equal)
  VLT  = 0xC    // [  v<  ]  :   -vlt           (vol lower-than)
};

SWC_CAN_INLINE 
Comp from(const char** buf, uint32_t* remainp, bool extended=false) {
  Comp comp = Comp::NONE;

  if(extended && *remainp > 2) {
    if(strncasecmp(*buf, "v>=", 3) == 0 || 
       strncasecmp(*buf, "vge", 3) == 0)
      comp = Comp::VGE;
    else if(strncasecmp(*buf, "v<=", 3) == 0 || 
            strncasecmp(*buf, "vle", 3) == 0)
      comp = Comp::VLE;
    else if(strncasecmp(*buf, "vgt", 3) == 0)
      comp = Comp::VGT;
    else if(strncasecmp(*buf, "vlt", 3) == 0)
      comp = Comp::VLT;
    
    if(comp != Comp::NONE) {
      *buf += 3;
      *remainp -= 3;
      return comp;
    }
  }

  if(*remainp > 1) {
    if(strncasecmp(*buf, "=^", 2) == 0 || strncasecmp(*buf, "pf", 2) == 0)
      comp = Comp::PF;
    else if(strncasecmp(*buf, ">=", 2) == 0 || strncasecmp(*buf, "ge", 2) == 0)
      comp = Comp::GE;
    else if(strncasecmp(*buf, "<=", 2) == 0 || strncasecmp(*buf, "le", 2) == 0)
      comp = Comp::LE;
    else if(strncasecmp(*buf, "!=", 2) == 0 || strncasecmp(*buf, "ne", 2) == 0)
      comp = Comp::NE;
    else if(strncasecmp(*buf, "re", 2) == 0)
      comp = Comp::RE;
    else if(strncasecmp(*buf, "==", 2) == 0 || strncasecmp(*buf, "eq", 2) == 0)
      comp = Comp::EQ;
    else if(strncasecmp(*buf, "gt", 2) == 0)
      comp = Comp::GT;
    else if(strncasecmp(*buf, "lt", 2) == 0)
      comp = Comp::LT;
    if(extended) {
      if(strncasecmp(*buf, "v>", 2) == 0)
        comp = Comp::VGT;
      else if(strncasecmp(*buf, "v<", 2) == 0)
        comp = Comp::VLT;
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
    else if(**buf == 'r' || **buf == 'R')
      comp = Comp::RE;

    if(comp != Comp::NONE) {
      *buf += 1;
      *remainp -= 1;
      return comp;
    }
  }

  return comp;
};

SWC_CAN_INLINE 
std::string to_string(Comp comp, bool extended=false) {
  
  if(extended) switch (comp) {
    case Comp::VGT:
      return std::string("v>");
    case Comp::VGE:
      return std::string("v>=");
    case Comp::VLE:
      return std::string("v<=");
    case Comp::VLT:
      return std::string("v<");
    default:
      break;
  }

  switch (comp) {
    case Comp::EQ:
      return std::string("==");
    case Comp::PF:
      return std::string("=^");
    case Comp::GT:
      return std::string(">");
    case Comp::GE:
      return std::string(">=");
    case Comp::LE:
      return std::string("<=");
    case Comp::LT:
      return std::string("<");
    case Comp::NE:
      return std::string("!=");
    case Comp::RE:
     return std::string("RE");
    default:
      return std::string("none");
  }
};

SWC_CAN_INLINE 
std::string to_string(uint8_t comp) {
  return to_string((Comp)comp);
};



SWC_CAN_INLINE 
Comp condition_lexic(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len ? p1_len: p2_len);
  return diff == 0
          ? (p1_len == p2_len 
            ? Comp::EQ 
            : (p1_len < p2_len ? Comp::GT : Comp::LT)
            )
          : (diff < 0 ? Comp::GT : Comp::LT)
          ;
}

SWC_CAN_INLINE 
Comp condition_volume(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  int diff;
  return (
    p1_len < p2_len 
    ? Comp::GT 
    : (p1_len > p2_len 
      ? Comp::LT
      : ((diff = memcmp(p1, p2, p1_len)) == 0 
        ? Comp::EQ 
        : (diff < 0 
          ? Comp::GT 
          : Comp::LT
          )
        ) 
      )
    );
}



SWC_CAN_INLINE 
Comp condition(bool vol, const uint8_t *p1, uint32_t p1_len, 
                         const uint8_t *p2, uint32_t p2_len) {
  return (vol ? condition_volume : condition_lexic)
        (p1, p1_len, p2, p2_len);
}

SWC_CAN_INLINE 
bool pf(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) {
  return p1_len <= p2_len && memcmp(p1, p2, p1_len) == 0;
}

SWC_CAN_INLINE 
bool gt_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff < 0 || (diff == 0 && p1_len < p2_len);
}

SWC_CAN_INLINE 
bool gt_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) {
  return p1_len < p2_len || (p1_len == p2_len && memcmp(p1, p2, p2_len) < 0);
}

SWC_CAN_INLINE 
bool ge_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff < 0 || (diff == 0 && p1_len <= p2_len);
}

SWC_CAN_INLINE 
bool ge_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) {
  return p1_len < p2_len || (p1_len == p2_len && memcmp(p1, p2, p2_len) <= 0);
}

SWC_CAN_INLINE 
bool eq(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) {
  return p1_len == p2_len && memcmp(p1, p2, p1_len) == 0;
}

SWC_CAN_INLINE 
bool le_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff > 0 || (diff == 0 && p1_len >= p2_len);
}

SWC_CAN_INLINE 
bool le_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) {
  return p1_len > p2_len || (p1_len == p2_len && memcmp(p1, p2, p1_len) >= 0);
}

SWC_CAN_INLINE 
bool lt_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff > 0 || (diff == 0 && p1_len > p2_len);
} 

SWC_CAN_INLINE 
bool lt_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) {
  return p1_len > p2_len || (p1_len == p2_len && memcmp(p1, p2, p1_len) > 0);
} 

SWC_CAN_INLINE 
bool ne(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) {
  return !eq(p1, p1_len, p2, p2_len);
}

SWC_CAN_INLINE 
bool re(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) {
  return re2::RE2::PartialMatch(
    re2::StringPiece((const char *)p2, p2_len), 
    re2::RE2(re2::StringPiece((const char *)p1, p1_len))
  );
}

SWC_CAN_INLINE 
bool re(const RE2* regex, const uint8_t *p2, uint32_t p2_len) {
  return RE2::PartialMatch(
    re2::StringPiece((const char *)p2, p2_len), 
    *regex
  );
}


SWC_CAN_INLINE 
bool is_matching_lexic(uint8_t comp, 
                       const uint8_t *p1, uint32_t p1_len, 
                       const uint8_t *p2, uint32_t p2_len) {
  switch (comp) {

    case Comp::PF:
      return pf(p1, p1_len, p2, p2_len);

    case Comp::GT:
      return gt_lexic(p1, p1_len, p2, p2_len);

    case Comp::GE:
      return ge_lexic(p1, p1_len, p2, p2_len);

    case Comp::EQ:
      return eq(p1, p1_len, p2, p2_len);

    case Comp::LE:
      return le_lexic(p1, p1_len, p2, p2_len);

    case Comp::LT:
      return lt_lexic(p1, p1_len, p2, p2_len);

    case Comp::NE:
      return ne(p1, p1_len, p2, p2_len);

    case Comp::RE:
      return re(p1, p1_len, p2, p2_len);

    default:
      return true;
  }
}

SWC_CAN_INLINE 
bool is_matching_volume(uint8_t comp, 
                        const uint8_t *p1, uint32_t p1_len, 
                        const uint8_t *p2, uint32_t p2_len) {
  switch (comp) {

    case Comp::PF:
      return pf(p1, p1_len, p2, p2_len);

    case Comp::GT:
      return gt_volume(p1, p1_len, p2, p2_len);

    case Comp::GE:
      return ge_volume(p1, p1_len, p2, p2_len);

    case Comp::EQ:
      return eq(p1, p1_len, p2, p2_len);

    case Comp::LE:
      return le_volume(p1, p1_len, p2, p2_len);

    case Comp::LT:
      return lt_volume(p1, p1_len, p2, p2_len);

    case Comp::NE:
      return ne(p1, p1_len, p2, p2_len);

    case Comp::RE:
      return re(p1, p1_len, p2, p2_len);

    default:
      return true;
  }
}


SWC_CAN_INLINE 
bool is_matching_lexic(uint8_t comp, 
                       const char *p1, uint32_t p1_len, 
                       const char *p2, uint32_t p2_len) {
  return is_matching_lexic(
    comp, (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}

SWC_CAN_INLINE 
bool is_matching_volume(uint8_t comp, 
                        const char *p1, uint32_t p1_len, 
                        const char *p2, uint32_t p2_len) {
  return is_matching_volume(
    comp, (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}


SWC_CAN_INLINE 
bool is_matching(bool volumetric, uint8_t comp, 
                 const uint8_t *p1, uint32_t p1_len, 
                 const uint8_t *p2, uint32_t p2_len) {
  return volumetric 
    ? is_matching_volume(comp, p1, p1_len, p2, p2_len)
    : is_matching_lexic(comp, p1, p1_len, p2, p2_len);
}

SWC_CAN_INLINE 
bool is_matching(bool volumetric, uint8_t comp, 
                 const char *p1, uint32_t p1_len, 
                 const char *p2, uint32_t p2_len) {
  return is_matching(volumetric, comp, 
                    (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}

SWC_CAN_INLINE 
bool is_matching_extended(uint8_t comp, 
                          const uint8_t *p1, uint32_t p1_len, 
                          const uint8_t *p2, uint32_t p2_len) {
  switch (comp) {

    case Comp::PF:
      return pf(p1, p1_len, p2, p2_len);

    case Comp::GT:
      return gt_lexic(p1, p1_len, p2, p2_len);

    case Comp::GE:
      return ge_lexic(p1, p1_len, p2, p2_len);

    case Comp::EQ:
      return eq(p1, p1_len, p2, p2_len);

    case Comp::LE:
      return le_lexic(p1, p1_len, p2, p2_len);

    case Comp::LT:
      return lt_lexic(p1, p1_len, p2, p2_len);

    case Comp::NE:
      return ne(p1, p1_len, p2, p2_len);

    case Comp::RE:
      return re(p1, p1_len, p2, p2_len);

    case Comp::VGT:
      return gt_volume(p1, p1_len, p2, p2_len);

    case Comp::VGE:
      return ge_volume(p1, p1_len, p2, p2_len);

    case Comp::VLE:
      return le_volume(p1, p1_len, p2, p2_len);

    case Comp::VLT:
      return lt_volume(p1, p1_len, p2, p2_len);

    default:
      return true;
  }
}


// const int64_t

SWC_CAN_INLINE 
bool gt(const int64_t p1, const int64_t p2) {
  return p1 < p2;
}

SWC_CAN_INLINE 
bool ge(const int64_t p1, const int64_t p2) {
  return p1 <= p2;
}

SWC_CAN_INLINE 
bool eq(const int64_t p1, const int64_t p2) {
  return p1 == p2;
}

SWC_CAN_INLINE 
bool le(const int64_t p1, const int64_t p2) {
  return p1 >= p2;
}

SWC_CAN_INLINE 
bool lt(const int64_t p1, const int64_t p2) {
  return p1 > p2;
}

SWC_CAN_INLINE 
bool ne(const int64_t p1, const int64_t p2) {
  return p1 != p2;
}

SWC_CAN_INLINE 
bool is_matching(uint8_t comp, const int64_t p1, const int64_t p2) {
  switch (comp) {

    case Comp::GT:
      return gt(p1, p2);

    case Comp::GE:
      return ge(p1, p2);

    case Comp::EQ:
      return eq(p1, p2);

    case Comp::LE:
      return le(p1, p2);

    case Comp::LT:
      return lt(p1, p2);

    case Comp::NE:
      return ne(p1, p2);

    default:
      return true;
  }
}



} }


# undef SWC_CAN_INLINE 
# undef SWC_NOINLINE 

#endif
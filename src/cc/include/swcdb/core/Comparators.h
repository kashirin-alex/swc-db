/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Comparators_h
#define swcdb_core_Comparators_h

#include "swcdb/core/Compat.h"
#include <cstring>
#include <re2/re2.h>



namespace SWC { 

/**
 * @brief The SWC-DB Comparators C++ namespace 'SWC::Condition'
 *
 * \ingroup Core
 */
namespace Condition {



enum Comp : uint8_t {
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



const char COMP_NONE[]  = "none";
const char COMP_PF[]    = "=^";
const char COMP_GT[]    = ">";
const char COMP_GE[]    = ">=";
const char COMP_EQ[]    = "==";
const char COMP_LE[]    = "<=";
const char COMP_LT[]    = "<";
const char COMP_NE[]    = "!=";
const char COMP_RE[]    = "re";
const char COMP_VGT[]   = "v>";
const char COMP_VGE[]   = "v>=";
const char COMP_VLE[]   = "v<=";
const char COMP_VLT[]   = "v<";



extern SWC_CAN_INLINE 
Comp from(const char** buf, uint32_t* remainp, bool extended=false) {
  Comp comp = Comp::NONE;

  if(extended && *remainp > 2) {
    if(!strncasecmp(*buf, COMP_VGE, 3) || 
       !strncasecmp(*buf, "vge", 3))
      comp = Comp::VGE;
    else if(!strncasecmp(*buf, COMP_VLE, 3) || 
            !strncasecmp(*buf, "vle", 3))
      comp = Comp::VLE;
    else if(!strncasecmp(*buf, "vgt", 3))
      comp = Comp::VGT;
    else if(!strncasecmp(*buf, "vlt", 3))
      comp = Comp::VLT;
    
    if(comp != Comp::NONE) {
      *buf += 3;
      *remainp -= 3;
      return comp;
    }
  }

  if(*remainp > 1) {
    if(!strncasecmp(*buf, COMP_PF, 2) || 
       !strncasecmp(*buf, "pf", 2))
      comp = Comp::PF;
    else if(!strncasecmp(*buf, COMP_GE, 2) || 
            !strncasecmp(*buf, "ge", 2))
      comp = Comp::GE;
    else if(!strncasecmp(*buf, COMP_LE, 2) || 
            !strncasecmp(*buf, "le", 2))
      comp = Comp::LE;
    else if(!strncasecmp(*buf, COMP_NE, 2) || 
            !strncasecmp(*buf, "ne", 2))
      comp = Comp::NE;
    else if(!strncasecmp(*buf, COMP_RE, 2))
      comp = Comp::RE;
    else if(!strncasecmp(*buf, COMP_EQ, 2) || 
            !strncasecmp(*buf, "eq", 2))
      comp = Comp::EQ;
    else if(!strncasecmp(*buf, "gt", 2))
      comp = Comp::GT;
    else if(!strncasecmp(*buf, "lt", 2))
      comp = Comp::LT;
    if(extended) {
      if(!strncasecmp(*buf, COMP_VGT, 2))
        comp = Comp::VGT;
      else if(!strncasecmp(*buf, COMP_VLT, 2))
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

extern SWC_CAN_INLINE 
const char* to_string(Comp comp, bool extended=false) {
  
  if(extended) switch (comp) {
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
    default:
      return COMP_NONE;
  }
};

extern SWC_CAN_INLINE 
const char* to_string(uint8_t comp) {
  return to_string((Comp)comp);
};


namespace { // local namespace
static int
_memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept
  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static int
_memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2)
    if(*s1 != *s2) 
      return *s1 < *s2 ? -1 : 1;
  return 0;
}
}

// performance equal to builtin memcmp
extern int
memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept
  __attribute__((optimize("-O3")));

extern SWC_CAN_INLINE   
int
memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept {
  return _memcomp(s1, s2, count);
}



extern SWC_CAN_INLINE 
Comp condition_lexic(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = memcmp(p1, p2, p1_len < p2_len ? p1_len: p2_len);
  return !diff
          ? (p1_len == p2_len 
            ? Comp::EQ 
            : (p1_len < p2_len ? Comp::GT : Comp::LT)
            )
          : (diff < 0 ? Comp::GT : Comp::LT)
          ;
}

extern SWC_CAN_INLINE 
Comp condition_volume(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff;
  return (
    p1_len < p2_len 
    ? Comp::GT 
    : (p1_len > p2_len 
      ? Comp::LT
      : (!(diff = memcmp(p1, p2, p1_len))
        ? Comp::EQ 
        : (diff < 0 
          ? Comp::GT 
          : Comp::LT
          )
        ) 
      )
    );
}



extern SWC_CAN_INLINE 
Comp condition(bool vol, const uint8_t *p1, uint32_t p1_len, 
                         const uint8_t *p2, uint32_t p2_len) noexcept {
  return (vol ? condition_volume : condition_lexic)
        (p1, p1_len, p2, p2_len);
}

extern SWC_CAN_INLINE 
bool pf(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len <= p2_len && !memcmp(p1, p2, p1_len);
}

extern SWC_CAN_INLINE 
bool gt_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff < 0 || (!diff && p1_len < p2_len);
}

extern SWC_CAN_INLINE 
bool gt_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) noexcept{
  return p1_len < p2_len || (p1_len == p2_len && memcmp(p1, p2, p2_len) < 0);
}

extern SWC_CAN_INLINE 
bool ge_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff < 0 || (!diff && p1_len <= p2_len);
}

extern SWC_CAN_INLINE 
bool ge_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len < p2_len || (p1_len == p2_len && memcmp(p1, p2, p2_len) <= 0);
}

extern SWC_CAN_INLINE 
bool eq(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len == p2_len && !memcmp(p1, p2, p1_len);
}

extern SWC_CAN_INLINE 
bool le_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff > 0 || (!diff && p1_len >= p2_len);
}

extern SWC_CAN_INLINE 
bool le_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len > p2_len || (p1_len == p2_len && memcmp(p1, p2, p1_len) >= 0);
}

extern SWC_CAN_INLINE 
bool lt_lexic(const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff > 0 || (!diff && p1_len > p2_len);
} 

extern SWC_CAN_INLINE 
bool lt_volume(const uint8_t *p1, uint32_t p1_len, 
               const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len > p2_len || (p1_len == p2_len && memcmp(p1, p2, p1_len) > 0);
} 

extern SWC_CAN_INLINE 
bool ne(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) noexcept {
  return !eq(p1, p1_len, p2, p2_len);
}

extern SWC_CAN_INLINE 
bool re(const uint8_t *p1, uint32_t p1_len, 
        const uint8_t *p2, uint32_t p2_len) {
  return re2::RE2::PartialMatch(
    re2::StringPiece((const char *)p2, p2_len), 
    re2::RE2(re2::StringPiece((const char *)p1, p1_len))
  );
}

extern SWC_CAN_INLINE 
bool re(const RE2* regex, const uint8_t *p2, uint32_t p2_len) {
  return RE2::PartialMatch(
    re2::StringPiece((const char *)p2, p2_len), 
    *regex
  );
}


extern SWC_CAN_INLINE 
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

extern SWC_CAN_INLINE 
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


extern SWC_CAN_INLINE 
bool is_matching_lexic(uint8_t comp, 
                       const char *p1, uint32_t p1_len, 
                       const char *p2, uint32_t p2_len) {
  return is_matching_lexic(
    comp, (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}

extern SWC_CAN_INLINE 
bool is_matching_volume(uint8_t comp, 
                        const char *p1, uint32_t p1_len, 
                        const char *p2, uint32_t p2_len) {
  return is_matching_volume(
    comp, (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}


extern SWC_CAN_INLINE 
bool is_matching(bool volumetric, uint8_t comp, 
                 const uint8_t *p1, uint32_t p1_len, 
                 const uint8_t *p2, uint32_t p2_len) {
  return volumetric 
    ? is_matching_volume(comp, p1, p1_len, p2, p2_len)
    : is_matching_lexic(comp, p1, p1_len, p2, p2_len);
}

extern SWC_CAN_INLINE 
bool is_matching(bool volumetric, uint8_t comp, 
                 const char *p1, uint32_t p1_len, 
                 const char *p2, uint32_t p2_len) {
  return is_matching(volumetric, comp, 
                    (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}

extern SWC_CAN_INLINE 
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

extern SWC_CAN_INLINE 
bool gt(const int64_t p1, const int64_t p2) {
  return p1 < p2;
}

extern SWC_CAN_INLINE 
bool ge(const int64_t p1, const int64_t p2) {
  return p1 <= p2;
}

extern SWC_CAN_INLINE 
bool eq(const int64_t p1, const int64_t p2) {
  return p1 == p2;
}

extern SWC_CAN_INLINE 
bool le(const int64_t p1, const int64_t p2) {
  return p1 >= p2;
}

extern SWC_CAN_INLINE 
bool lt(const int64_t p1, const int64_t p2) {
  return p1 > p2;
}

extern SWC_CAN_INLINE 
bool ne(const int64_t p1, const int64_t p2) {
  return p1 != p2;
}

extern SWC_CAN_INLINE 
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



} } // namespace SWC::Condition




#endif // swcdb_core_Comparators_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Comparators_h
#define swcdb_core_Comparators_h

#include "swcdb/core/Compat.h"
#include <cstring>
#include <cmath>
#include <re2/re2.h>



namespace SWC {

/**
 * @brief The SWC-DB Comparators C++ namespace 'SWC::Condition'
 *
 * \ingroup Core
 */
namespace Condition {



enum Comp : uint8_t {
  NONE    = 0x00, // [      ] :  -none            (no comparison aplied)
  PF      = 0x01, // [  =^  ] :  -pf [prefix]     (starts-with)
  GT      = 0x02, // [  >   ] :  -gt              (greater-than)
  GE      = 0x03, // [  >=  ] :  -ge              (greater-equal)
  EQ      = 0x04, // [  =   ] :  -eq              (equal)
  LE      = 0x05, // [  <=  ] :  -le              (lower-equal)
  LT      = 0x06, // [  <   ] :  -lt              (lower-than)
  NE      = 0x07, // [  !=  ] :  -ne              (not-equal)
  RE      = 0x08, // [  re  ] :  -re [r,regexp]   (regular-expression)

  // extended logic options: ge,le,gt,lt are LEXIC and with 'V' VOLUME
  VGT     = 0x09, // [  v>  ] :  -vgt              (vol greater-than)
  VGE     = 0x0A, // [  v>= ] :  -vge              (vol greater-equal)
  VLE     = 0x0B, // [  v<= ] :  -vle              (vol lower-equal)
  VLT     = 0x0C, // [  v<  ] :  -vlt              (vol lower-than)

  //
  SBS     = 0x0D, // [  %>  ] :  -subset [sbs]     (subset)
  SPS     = 0x0E, // [  <%  ] :  -supset [sps]     (superset)
  POSBS   = 0x0F, // [  ~>  ] :  -posubset [posbs] (eq/part ordered subset)
  POSPS   = 0x10, // [  <~  ] :  -posupset [posps] (eq/part ordered superset)
  FOSBS   = 0x11, // [  ->  ] :  -fosubset [fosbs] (eq/full ordered subset)
  FOSPS   = 0x12, // [  <-  ] :  -fosupset [fosps] (eq/full ordered superset)

  /*  p1(spec) p2(data)
    SUBSET:
      int/double - (p2 <% p1) True is ((p1 mod p2) == 0)
      domain object True is (p2[1,2,3] <% p1[6,3,2,1,9])
    SUPSET:
      int/double - (p2 %> p1) True is ((p2 mod p1) == 0)
      domain object True is (p2[6,3,2,1,9] %> p1[1,2,3])

    // Partially Ordered (int/double not supported)
    PO-SUBSET:
      domain object True is (p2[1,2,3] '<~' p1[6,1,321,2,9,3,9])
    PO-SUPSET:
      domain object True is (p2[6,1,321,2,9,3,9] '~>' p1[1,2,3])

    // Fully Ordered (double not supported)
    FO-SUBSET:
      int - (p2 <- p1) True is ((p1 OR p2) == p1)
      domain object True is (p2[1,2,3] '<-' p1[6,1,2,3,9])
    FO-SUPSET:
      int - (p2 -> p1) True is ((p2 OR p1) == p2)
      domain object True is (p2[6,1,2,3,9] '->' p1[1,2,3])
  */

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
const char COMP_SBS[]   = "%>";
const char COMP_SPS[]   = "<%";
const char COMP_POSBS[] = "~>";
const char COMP_POSPS[] = "<~";
const char COMP_FOSBS[] = "->";
const char COMP_FOSPS[] = "<-";



extern SWC_CAN_INLINE
Comp from(const char** buf, uint32_t* remainp,
          bool extended=false)  noexcept {
  Comp comp = Comp::NONE;

  if(*remainp > 7) {
    if(!strncasecmp(*buf, "posubset", 8))
      comp = Comp::POSBS;
    else if(!strncasecmp(*buf, "posupset", 8))
      comp = Comp::POSPS;
    else if(!strncasecmp(*buf, "fosubset", 8))
      comp = Comp::FOSBS;
    else if(!strncasecmp(*buf, "fosupset", 8))
      comp = Comp::FOSPS;

    if(comp != Comp::NONE) {
      *buf += 8;
      *remainp -= 8;
      return comp;
    }
  }

  if(*remainp > 5) {
    if(!strncasecmp(*buf, "subset", 6))
      comp = Comp::SBS;
    else if(!strncasecmp(*buf, "supset", 6))
      comp = Comp::SPS;

    if(comp != Comp::NONE) {
      *buf += 6;
      *remainp -= 6;
      return comp;
    }
  }

  if(*remainp > 4) {
    if(!strncasecmp(*buf, "posbs", 5))
      comp = Comp::POSBS;
    else if(!strncasecmp(*buf, "posps", 5))
      comp = Comp::POSPS;
    else if(!strncasecmp(*buf, "fosbs", 5))
      comp = Comp::FOSBS;
    else if(!strncasecmp(*buf, "fosps", 5))
      comp = Comp::FOSPS;

    if(comp != Comp::NONE) {
      *buf += 5;
      *remainp -= 5;
      return comp;
    }
  }

  if(*remainp > 2) {
    if(!strncasecmp(*buf, "sbs", 3)) {
      comp = Comp::SBS;
    } else if(!strncasecmp(*buf, "sps", 3)) {
      comp = Comp::SPS;
    } else if(extended) {
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
    }
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
    else if(!strncasecmp(*buf, COMP_SBS, 2))
      comp = Comp::SBS;
    else if(!strncasecmp(*buf, COMP_SPS, 2))
      comp = Comp::SPS;
    else if(!strncasecmp(*buf, COMP_POSBS, 2))
      comp = Comp::POSBS;
    else if(!strncasecmp(*buf, COMP_POSPS, 2))
      comp = Comp::POSPS;
    else if(!strncasecmp(*buf, COMP_FOSBS, 2))
      comp = Comp::FOSBS;
    else if(!strncasecmp(*buf, COMP_FOSPS, 2))
      comp = Comp::FOSPS;

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
}

extern SWC_CAN_INLINE
const char* to_string(Comp comp, bool extended=false) noexcept {

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

extern SWC_CAN_INLINE
const char* to_string(uint8_t comp) {
  return to_string((Comp)comp);
}


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


static int
_strncomp(const char* s1, const char* s2, size_t count) noexcept
  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static int
_strncomp(const char* s1, const char* s2, size_t count) noexcept {
  for(uint8_t b1, b2; count; --count, ++s1, ++s2) {
    if((b1 = *s1) != (b2 = *s2))
      return b1 < b2 ? -1 : 1;
    if(!b1)
      break;
  }
  return 0;
}


static int
_strcomp(const char* s1, const char* s2) noexcept
  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static int
_strcomp(const char* s1, const char* s2) noexcept {
  for(uint8_t b1, b2; ; ++s1, ++s2) {
    if((b1 = *s1) != (b2 = *s2))
      return b1 < b2 ? -1 : 1;
    if(!b1)
      break;
  }
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

extern int
strncomp(const char* s1, const char* s2, size_t count) noexcept
  __attribute__((optimize("-O3")));

extern SWC_CAN_INLINE
int
strncomp(const char* s1, const char* s2, size_t count) noexcept {
  return _strncomp(s1, s2, count);
}

extern int
strcomp(const char* s1, const char* s2) noexcept
  __attribute__((optimize("-O3")));

extern SWC_CAN_INLINE
int
strcomp(const char* s1, const char* s2) noexcept {
  return _strcomp(s1, s2);
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
bool ne(const uint8_t* p1, uint32_t p1_len,
        const uint8_t* p2, uint32_t p2_len) noexcept {
  return !eq(p1, p1_len, p2, p2_len);
}

extern SWC_CAN_INLINE
bool re(const re2::RE2& regex, const char* v, uint32_t v_len) {
  return re2::RE2::PartialMatch(re2::StringPiece(v, v_len), regex);
}

extern SWC_CAN_INLINE
bool re(const uint8_t* p1, uint32_t p1_len,
        const uint8_t* p2, uint32_t p2_len) {
  return re(
    re2::RE2(re2::StringPiece((const char *)p1, p1_len)),
    (const char *)p2, p2_len
  );
}

extern SWC_CAN_INLINE
bool sbs(const uint8_t* p1, uint32_t p1_len,
         const uint8_t* p2, uint32_t p2_len) noexcept {
  if(p1_len > p2_len)
    return false;
  std::vector<const uint8_t*> found(p1_len, nullptr);
  const uint8_t* p2_end = p2 + p2_len;
  for(const uint8_t* p1_end = p1 + p1_len; p1 < p1_end; ++p1) {
    for(const uint8_t* ptr = p2; ; ) {
      if(*p1 == *ptr) {
        auto it = found.begin();
        for(; *it; ++it) {
          if(*it == ptr)
            goto _continue;
        }
        *it = ptr;
        break;
      }
      _continue:
        if(++ptr == p2_end)
          return false;
    }

  }
  return true;
}

extern SWC_CAN_INLINE
bool po_sbs(const uint8_t* p1, uint32_t p1_len,
            const uint8_t* p2, uint32_t p2_len) noexcept {
  if(p1_len > p2_len)
    return false;
  const uint8_t* p1_end = p1 + p1_len;
  const uint8_t* p2_end = p2 + p2_len;
  for(; p1 < p1_end && p2 < p2_end; ++p2) {
    if(*p1 == *p2)
      ++p1;
  }
  return p1 == p1_end;
}

extern SWC_CAN_INLINE
bool fo_sbs(const uint8_t* p1, uint32_t p1_len,
            const uint8_t* p2, uint32_t p2_len) noexcept {
  if(p1_len > p2_len)
    return false;
  bool start = false;
  const uint8_t* p1_end = p1 + p1_len;
  const uint8_t* p2_end = p2 + p2_len;
  for(; p1 < p1_end && p2 < p2_end; ++p2) {
    if(*p1 == *p2) {
      start = true;
      ++p1;
    } else if(start) {
      return false;
    }
  }
  return p1 == p1_end;
}

extern SWC_CAN_INLINE
bool is_matching_lexic(uint8_t comp,
                       const uint8_t* p1, uint32_t p1_len,
                       const uint8_t* p2, uint32_t p2_len) {
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

    case Comp::SBS:
      return sbs(p1, p1_len, p2, p2_len);

    case Comp::SPS:
      return sbs(p2, p2_len, p1, p1_len);

    case Comp::POSBS:
      return po_sbs(p1, p1_len, p2, p2_len);

    case Comp::POSPS:
      return po_sbs(p2, p2_len, p1, p1_len);

    case Comp::FOSBS:
      return fo_sbs(p1, p1_len, p2, p2_len);

    case Comp::FOSPS:
      return fo_sbs(p2, p2_len, p1, p1_len);

    default:
      return true;
  }
}

extern SWC_CAN_INLINE
bool is_matching_volume(uint8_t comp,
                        const uint8_t* p1, uint32_t p1_len,
                        const uint8_t* p2, uint32_t p2_len) {
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

    case Comp::SBS:
      return sbs(p1, p1_len, p2, p2_len);

    case Comp::SPS:
      return sbs(p2, p2_len, p1, p1_len);

    case Comp::POSBS:
      return po_sbs(p1, p1_len, p2, p2_len);

    case Comp::POSPS:
      return po_sbs(p2, p2_len, p1, p1_len);

    case Comp::FOSBS:
      return fo_sbs(p1, p1_len, p2, p2_len);

    case Comp::FOSPS:
      return fo_sbs(p2, p2_len, p1, p1_len);


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

    case Comp::SBS:
      return sbs(p1, p1_len, p2, p2_len);

    case Comp::SPS:
      return sbs(p2, p2_len, p1, p1_len);

    case Comp::POSBS:
      return po_sbs(p1, p1_len, p2, p2_len);

    case Comp::POSPS:
      return po_sbs(p2, p2_len, p1, p1_len);

    case Comp::FOSBS:
      return fo_sbs(p1, p1_len, p2, p2_len);

    case Comp::FOSPS:
      return fo_sbs(p2, p2_len, p1, p1_len);

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


// const T

template<typename T>
extern SWC_CAN_INLINE
bool gt(const T p1, const T p2) noexcept {
  return p1 < p2;
}

template<typename T>
extern SWC_CAN_INLINE
bool ge(const T p1, const T p2) noexcept {
  return p1 <= p2;
}

template<typename T>
extern SWC_CAN_INLINE
bool eq(const T p1, const T p2) noexcept {
  return p1 == p2;
}

template<typename T>
extern SWC_CAN_INLINE
bool le(const T p1, const T p2) noexcept {
  return p1 >= p2;
}

template<typename T>
extern SWC_CAN_INLINE
bool lt(const T p1, const T p2) noexcept {
  return p1 > p2;
}

template<typename T>
extern SWC_CAN_INLINE
bool ne(const T p1, const T p2) noexcept {
  return p1 != p2;
}

template<typename T>
extern SWC_CAN_INLINE
bool sbs(const T p1, const T p2) noexcept {
  return (p2 % p1) == 0;
}

template<>
SWC_CAN_INLINE
bool sbs(const long double p1, const long double p2) noexcept {
  return fmod(p2, p1) == 0;
}

template<typename T>
extern SWC_CAN_INLINE
bool fo_sbs(const T p1, const T p2) noexcept {
  return (p2 | p1) == p2;
}

template<>
SWC_CAN_INLINE
bool fo_sbs(const long double, const long double) noexcept {
  /* Not Implemented ?
  if(((uint64_t)p2 | (uint64_t)p1) != (uint64_t)p2)
    return false;
  auto t1 = p1;
  auto t2 = p2;
  for(;;t1 *= 10, t2 *= 10) {
    if(!fmod(t1, 10.0) && !fmod(t2, 10.0))
      return ((uint64_t)t2 | (uint64_t)t1) == (uint64_t)t2;
  }
  */
  return false;
}

template<typename T>
extern SWC_CAN_INLINE
bool is_matching(uint8_t comp, const T p1, const T p2) noexcept {
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

    case Comp::SBS:
      return sbs(p1, p2);

    case Comp::SPS:
      return sbs(p2, p1);

    case Comp::FOSBS:
      return fo_sbs(p1, p2);

    case Comp::FOSPS:
      return fo_sbs(p2, p1);

    default:
      return true;
  }
}



} } // namespace SWC::Condition




#endif // swcdb_core_Comparators_h

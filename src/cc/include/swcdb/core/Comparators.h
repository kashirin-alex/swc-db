/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Comparators_h
#define swcdb_core_Comparators_h

#include "swcdb/core/Compat.h"
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



Comp from(const char** buf, uint32_t* remainp, bool extended=false)  noexcept;

const char* to_string(Comp comp, bool extended=false) noexcept;

extern SWC_CAN_INLINE
const char* to_string(uint8_t comp) {
  return to_string(Comp(comp));
}




extern SWC_CAN_INLINE
Comp condition_lexic(const uint8_t *p1, uint32_t p1_len,
                     const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = mem_cmp(p1, p2, p1_len < p2_len ? p1_len: p2_len);
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
      : (!(diff = mem_cmp(p1, p2, p1_len))
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
  return p1_len <= p2_len && mem_eq(p1, p2, p1_len);
}

extern SWC_CAN_INLINE
bool gt_lexic(const uint8_t *p1, uint32_t p1_len,
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = mem_cmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff < 0 || (!diff && p1_len < p2_len);
}

extern SWC_CAN_INLINE
bool gt_volume(const uint8_t *p1, uint32_t p1_len,
               const uint8_t *p2, uint32_t p2_len) noexcept{
  return p1_len < p2_len ||
        (p1_len == p2_len && mem_cmp(p1, p2, p2_len) < 0);
}

extern SWC_CAN_INLINE
bool ge_lexic(const uint8_t *p1, uint32_t p1_len,
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = mem_cmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff < 0 || (!diff && p1_len <= p2_len);
}

extern SWC_CAN_INLINE
bool ge_volume(const uint8_t *p1, uint32_t p1_len,
               const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len < p2_len ||
        (p1_len == p2_len && mem_cmp(p1, p2, p2_len) <= 0);
}

extern SWC_CAN_INLINE
bool eq(const uint8_t *p1, uint32_t p1_len,
        const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len == p2_len && mem_eq(p1, p2, p1_len);
}

extern SWC_CAN_INLINE
bool le_lexic(const uint8_t *p1, uint32_t p1_len,
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = mem_cmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff > 0 || (!diff && p1_len >= p2_len);
}

extern SWC_CAN_INLINE
bool le_volume(const uint8_t *p1, uint32_t p1_len,
               const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len > p2_len ||
        (p1_len == p2_len && mem_cmp(p1, p2, p1_len) >= 0);
}

extern SWC_CAN_INLINE
bool lt_lexic(const uint8_t *p1, uint32_t p1_len,
              const uint8_t *p2, uint32_t p2_len) noexcept {
  int diff = mem_cmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff > 0 || (!diff && p1_len > p2_len);
}

extern SWC_CAN_INLINE
bool lt_volume(const uint8_t *p1, uint32_t p1_len,
               const uint8_t *p2, uint32_t p2_len) noexcept {
  return p1_len > p2_len ||
        (p1_len == p2_len && mem_cmp(p1, p2, p1_len) > 0);
}

extern SWC_CAN_INLINE
bool ne(const uint8_t* p1, uint32_t p1_len,
        const uint8_t* p2, uint32_t p2_len) noexcept {
  return !eq(p1, p1_len, p2, p2_len);
}


bool re(const re2::RE2& regex, const re2::StringPiece& value);

extern SWC_CAN_INLINE
bool re(const re2::RE2& regex, const char* v, uint32_t v_len) {
  return v && v_len && re(regex, re2::StringPiece(v, v_len));
}

extern SWC_CAN_INLINE
bool re(const uint8_t* p1, uint32_t p1_len,
        const uint8_t* p2, uint32_t p2_len) {
  if(!p1 || !p1_len)
    return !p2 || !p2_len;
  return re(
    re2::RE2(
      re2::StringPiece(reinterpret_cast<const char*>(p1), p1_len)),
    reinterpret_cast<const char*>(p2), p2_len
  );
}

extern SWC_CAN_INLINE
bool sbs(const uint8_t* p1, uint32_t p1_len,
         const uint8_t* p2, uint32_t p2_len) noexcept {
  if(!p1_len)
    return true;
  if(p1_len > p2_len)
    return false;
  Core::Vector<bool> found(p1_len, false);
  for(uint32_t count = p1_len; p2_len; ++p2, --p2_len) {
    for(uint32_t i = 0; i < p1_len; ++i) {
      if(!found[i] && p1[i] == *p2) {
        if(!--count)
          return true;
        found[i] = true;
        break;
      }
    }
  }
  return false;
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
    comp,
    reinterpret_cast<const uint8_t*>(p1), p1_len,
    reinterpret_cast<const uint8_t*>(p2), p2_len
  );
}

extern SWC_CAN_INLINE
bool is_matching_volume(uint8_t comp,
                        const char *p1, uint32_t p1_len,
                        const char *p2, uint32_t p2_len) {
  return is_matching_volume(
    comp,
    reinterpret_cast<const uint8_t*>(p1), p1_len,
    reinterpret_cast<const uint8_t*>(p2), p2_len
  );
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
  return is_matching(
    volumetric, comp,
    reinterpret_cast<const uint8_t*>(p1), p1_len,
    reinterpret_cast<const uint8_t*>(p2), p2_len
  );
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
extern constexpr SWC_CAN_INLINE
bool gt(const T p1, const T p2) noexcept {
  return p1 < p2;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool ge(const T p1, const T p2) noexcept {
  return p1 <= p2;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool eq(const T p1, const T p2) noexcept {
  return p1 == p2;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool le(const T p1, const T p2) noexcept {
  return p1 >= p2;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool lt(const T p1, const T p2) noexcept {
  return p1 > p2;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool ne(const T p1, const T p2) noexcept {
  return p1 != p2;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool sbs(const T p1, const T p2) noexcept {
  return (p2 % p1) == 0;
}

template<>
SWC_CAN_INLINE
bool sbs(const long double p1, const long double p2) noexcept {
  return fmod(p2, p1) == 0;
}

template<typename T>
extern constexpr SWC_CAN_INLINE
bool fo_sbs(const T p1, const T p2) noexcept {
  return (p2 | p1) == p2;
}

template<>
constexpr SWC_CAN_INLINE
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


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Comparators.cc"
#endif


#endif // swcdb_core_Comparators_h

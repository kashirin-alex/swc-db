/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Comparators_h
#define swcdb_db_Comparators_h

#include <memory>
#include <cstring>
#include <re2/re2.h>

namespace SWC { namespace Condition {

enum Comp {
  NONE = 0x0,   // [    ]  :   none     (no comparison aplied)
  PF   = 0x1,   // [ =^ ]  :   prefix   (starts-with)
  GT   = 0x2,   // [ >  ]  :   -gt      (greater-than)
  GE   = 0x3,   // [ >= ]  :   -ge      (greater-equal)
  EQ   = 0x4,   // [ =  ]  :   -eq      (equal)
  LE   = 0x5,   // [ <= ]  :   -le      (lower-equal)
  LT   = 0x6,   // [ <  ]  :   -lt      (lower-than)
  NE   = 0x7,   // [ != ]  :   -ne      (not-equal)
  RE   = 0x8    // [ re ]  :   regexp   (regular-expression)
};

inline const std::string to_string(Comp comp) {
  switch (comp) {
    case Comp::NONE:
      return std::string("none");
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
      return std::string("==");
  }
};
inline const std::string to_string(uint8_t comp) {
  return to_string((Comp)comp);
};


// const char *

inline static bool pf(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  return p1_len <= p2_len && memcmp(p1, p2, p1_len) == 0;
}

inline static bool gt(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return (diff < 0 && p1_len <= p2_len) || (diff == 0 && p1_len < p2_len);
}

inline static bool ge(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff <= 0 && p1_len <= p2_len;
}

inline static bool eq(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  return p1_len == p2_len && memcmp(p1, p2, p1_len) == 0;
}

inline static bool le(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return diff >= 0 && p1_len >= p2_len;
}

inline static bool lt(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return (diff > 0 && p1_len >= p2_len) || (diff == 0 && p1_len > p2_len);
}

inline static bool ne(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) {
  return p1_len != p2_len || memcmp(p1, p2, p1_len) != 0;
}

inline static bool re(const char *p1, uint32_t p1_len, 
                      const char *p2, uint32_t p2_len) {
  return RE2::PartialMatch(p2, RE2(std::string(p1, p1_len)));
}

inline static bool re(std::shared_ptr<RE2> regex, 
                      const char *p2, uint32_t p2_len) {
  return RE2::PartialMatch(p2, *regex);
}

inline static bool is_matching(uint8_t comp, 
                               const uint8_t *p1, uint32_t p1_len, 
                               const uint8_t *p2, uint32_t p2_len) {
  switch (comp) {

    case Comp::PF:
      return pf(p1, p1_len, p2, p2_len);

    case Comp::GT:
      return gt(p1, p1_len, p2, p2_len);

    case Comp::GE:
      return ge(p1, p1_len, p2, p2_len);

    case Comp::EQ:
      return eq(p1, p1_len, p2, p2_len);

    case Comp::LE:
      return le(p1, p1_len, p2, p2_len);

    case Comp::LT:
      return lt(p1, p1_len, p2, p2_len);

    case Comp::NE:
      return ne(p1, p1_len, p2, p2_len);

    case Comp::RE:
      return re((const char *)p1, p1_len, (const char *)p2, p2_len);

    default:
      return true;
  }
}

inline static bool is_matching(uint8_t comp, 
                               const char *p1, uint32_t p1_len, 
                               const char *p2, uint32_t p2_len) {
  return is_matching(comp, (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}


// const int64_t

inline static bool gt(const int64_t p1, const int64_t p2) {
  return p1 < p2;
}

inline static bool ge(const int64_t p1, const int64_t p2) {
  return p1 <= p2;
}

inline static bool eq(const int64_t p1, const int64_t p2) {
  return p1 == p2;
}

inline static bool le(const int64_t p1, const int64_t p2) {
  return p1 >= p2;
}

inline static bool lt(const int64_t p1, const int64_t p2) {
  return p1 > p2;
}

inline static bool ne(const int64_t p1, const int64_t p2) {
  return p1 != p2;
}

inline static bool is_matching(uint8_t comp,   
                               const int64_t p1, const int64_t p2) {
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



}
}



#endif
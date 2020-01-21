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

inline const Comp from(const char** buf, uint32_t* remainp) {
  Comp comp = Comp::NONE;

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

inline const Comp condition(const uint8_t *p1, uint32_t p1_len, 
                            const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  if(p1_len == p2_len && diff == 0)
    return Comp::EQ;
  if((diff < 0 && p1_len <= p2_len) || (diff >= 0 && p1_len < p2_len))
    return Comp::GT;
  else
    return Comp::LT;
}

inline const bool pf(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  return p1_len <= p2_len && memcmp(p1, p2, p1_len) == 0;
}

inline const bool gt(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return (diff < 0 && p1_len <= p2_len) || (diff >= 0 && p1_len < p2_len);
}

inline const bool ge(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return (diff <= 0 && p1_len <= p2_len) || (diff > 0 && p1_len < p2_len);
}

inline const bool eq(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  return p1_len == p2_len && memcmp(p1, p2, p1_len) == 0;
}

inline const bool le(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return (diff >= 0 && p1_len >= p2_len) || (diff < 0 && p1_len > p2_len);
}

inline const bool lt(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  int diff = memcmp(p1, p2, p1_len < p2_len? p1_len: p2_len);
  return (diff > 0 && p1_len >= p2_len) || (diff <= 0 && p1_len > p2_len);
} 

inline const bool ne(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  return p1_len != p2_len || memcmp(p1, p2, p1_len) != 0;
}

inline const bool re(const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) {
  return re2::RE2::PartialMatch(
    re2::StringPiece((const char *)p2, p2_len), 
    re2::RE2(re2::StringPiece((const char *)p1, p1_len))
  );
}

inline const bool re(const RE2* regex, const uint8_t *p2, uint32_t p2_len) {
  return RE2::PartialMatch(
    re2::StringPiece((const char *)p2, p2_len), 
    *regex
  );
}

inline const bool is_matching(uint8_t comp, 
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
      return re(p1, p1_len, p2, p2_len);

    default:
      return true;
  }
}

inline const bool is_matching(uint8_t comp, 
                              const char *p1, uint32_t p1_len, 
                              const char *p2, uint32_t p2_len) {
  return is_matching(comp, (const uint8_t *)p1, p1_len, (const uint8_t *)p2, p2_len);
}


// const int64_t

inline const bool gt(const int64_t p1, const int64_t p2) {
  return p1 < p2;
}

inline const bool ge(const int64_t p1, const int64_t p2) {
  return p1 <= p2;
}

inline const bool eq(const int64_t p1, const int64_t p2) {
  return p1 == p2;
}

inline const bool le(const int64_t p1, const int64_t p2) {
  return p1 >= p2;
}

inline const bool lt(const int64_t p1, const int64_t p2) {
  return p1 > p2;
}

inline const bool ne(const int64_t p1, const int64_t p2) {
  return p1 != p2;
}

inline const bool is_matching(uint8_t comp,   
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
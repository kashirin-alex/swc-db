/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_include_Comparators_h
#define swcdb_include_Comparators_h

#include <iostream>
#include <cstring>

namespace SWC {

enum Comparator {
  NONE, // [    ]  :   iternal-none  (no comparison aplied)
  PF,   // [ =^ ]  :   prefix   (starts-with)
  GT,   // [ >  ]  :   -gt      (greater-than)
  GE,   // [ >= ]  :   -ge      (greater-equal)
  EQ,   // [ =  ]  :   -eq      (equal)
  LE,   // [ <= ]  :   -le      (lower-equal)
  LT,   // [ <  ]  :   -lt      (lower-than)
  NE,   // [ != ]  :   -ne      (not-equal)
  RE    // [ re ]  :   regexp   (regular-expression)
};
std::ostream &operator<<(std::ostream &os, Comparator comp);


class ComparatorBase {

    /* True for default matcher */
    public:
    ComparatorBase(){}
    virtual ~ComparatorBase(){}
    
    virtual bool is_matching(const char **p1, uint32_t p1_len,
                             const char **p2, uint32_t p2_len){
      // std::cout << "ComparatorBase - *p1:\"" << p1 << "\"p1:\"" << *p1 
      // << "\"-" << p1_len << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
      return true;
    }
    virtual bool is_matching(int64_t p1, int64_t p2){
      // std::cout << "ComparatorBase - p1:" << p1 << "|p2:\"" << p2 << "\n";
      return true;
    }

    ComparatorBase* get_matcher(Comparator comp);
};

namespace Condition{

inline static bool is_equal(const uint8_t *p1, uint32_t p1_len, 
                            const uint8_t *p2, uint32_t p2_len){
  return !(p1_len != p2_len || memcmp(p1, p2, p1_len));
}

}
}



#endif
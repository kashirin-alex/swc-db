/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "ComparatorsType.h"
#include <ostream>


namespace SWC {

std::ostream &operator<<(std::ostream &os, Comparator comp) {
  switch (comp)
    {
    case Comparator::NONE:
      os << "(none)";
      break;
    case Comparator::PF:
      os << "=^";
      break;
    case Comparator::GT:
      os << ">";
      break;
    case Comparator::GE:
      os << ">=";
      break;
    case Comparator::LE:
      os << "<=";
      break;
    case Comparator::LT:
      os << "<";
      break;
    case Comparator::NE:
      os << "!=";
      break;
    case Comparator::RE:
      os << "RE";
      break;
    default:
      os << "==";
    }
  return os;
};


ComparatorBase* ComparatorBase::get_matcher(const Comparator comp) {
  // std::cout << "get_matcher {" << comp << "}\n";
  switch (comp) {
    case Comparator::PF:
      return new ComparatorPrefix();
    case Comparator::GT:
      return new ComparatorGreater();
    case Comparator::GE:
      return new ComparatorGreaterEqual();
    case Comparator::EQ:
      return new ComparatorEqual();
    case Comparator::LE:
      return new ComparatorLowerEqual();
    case Comparator::LT:
      return new ComparatorLower();
    case Comparator::NE:
      return new ComparatorNotEqual();
    case Comparator::RE:
      return new ComparatorRegexp();
    default:
      return new ComparatorBase();
  }
};


bool ComparatorPrefix::is_matching(const char **p1, uint32_t p1_len, 
                                   const char **p2, uint32_t p2_len){      
  return p2_len >= p1_len && memcmp(*p1, *p2, p1_len) == 0;
}
/*
bool ComparatorPrefix::is_matching(const char *p1, uint32_t p1_len,
                                   const char *p2, uint32_t p2_len){      
  std::cout << "ComparatorPrefix - p1:\"" << p1 << "\"-" << p1_len << "|p2:\"" << p2 << "\"-" << p2_len << "\n";

  const char *prefix_end = p1 + p1_len;
  for (; p1 < prefix_end; ++p1,++p2)
    if (*p1 != *p2) return false;
  return true;
}
*/


bool ComparatorGreater::is_matching(const char **p1, uint32_t p1_len, 
                                    const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorGreater - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  int diff = memcmp(*p1, *p2, p1_len<p2_len?p1_len:p2_len);
  return (diff == 0 && p1_len < p2_len) || diff < 0;
}
bool ComparatorGreater::is_matching(int64_t p1, int64_t p2){
  // std::cout << "ComparatorGreater - p1:" << p1 << "|p2:" << p2 << "\n";
  return p1 < p2;
}


bool ComparatorGreaterEqual::is_matching(const char **p1, uint32_t p1_len, 
                                         const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorGreaterEqual - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  int diff = memcmp(*p1, *p2, p1_len<p2_len?p1_len:p2_len);
  return diff <= 0 || (diff == 0 && p1_len <= p2_len);
}
bool ComparatorGreaterEqual::is_matching(int64_t p1, int64_t p2){
  // std::cout << "ComparatorGreaterEqual - p1:" << p1 << "|p2:" << p2 << "\n";
  return p1 <= p2;
}


bool ComparatorEqual::is_matching(const char **p1, uint32_t p1_len, 
                                  const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorEqual - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  return !(p1_len != p2_len || memcmp(*p1, *p2, p1_len));
}
bool ComparatorEqual::is_matching(int64_t p1, int64_t p2){
  // std::cout << "ComparatorEqual - p1:" << p1 << "|p2:" << p2 << "\n";
  return p1 ==  p2;
}


bool ComparatorLowerEqual::is_matching(const char **p1, uint32_t p1_len, 
                                       const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorLowerEqual - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  int diff = memcmp(*p1, *p2, p1_len<p2_len?p1_len:p2_len);
  return diff > 0 || (diff == 0 && p1_len >= p2_len);
}
bool ComparatorLowerEqual::is_matching(int64_t p1, int64_t p2){
  // std::cout << "ComparatorLowerEqual - p1:" << p1 << "|p2:" << p2 << "\n";
  return p1 >= p2;
}


bool ComparatorLower::is_matching(const char **p1, uint32_t p1_len, 
                                  const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorLower - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  int diff = memcmp(*p1, *p2, p1_len<p2_len?p1_len:p2_len);
  return (diff == 0 && p1_len > p2_len) || diff > 0;
}
bool ComparatorLower::is_matching(int64_t p1, int64_t p2){
  // std::cout << "ComparatorLower - p1:" << p1 << "|p2:" << p2 << "\n";
  return p1 > p2;
}


bool ComparatorNotEqual::is_matching(const char **p1, uint32_t p1_len, 
                                     const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorNotEqual - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  return p1_len != p2_len || memcmp(*p1, *p2, p1_len) != 0;
}
bool ComparatorNotEqual::is_matching(int64_t p1, int64_t p2){
  // std::cout << "ComparatorNotEqual - p1:" << p1 << "|p2:" << p2 << "\n";
  return p1 != p2;
}


bool ComparatorRegexp::is_matching(const char **p1, uint32_t p1_len, 
                                   const char **p2, uint32_t p2_len){  
  // std::cout << "ComparatorRegexp - p1:\"" << *p1 << "\"-" << p1_len 
  //           << "|p2:\"" << *p2 << "\"-" << p2_len << "\n";
  if (!regex)
    regex.reset(new RE2(std::string(*p1, p1_len)));
  return RE2::PartialMatch(*p2, *regex);
}


}
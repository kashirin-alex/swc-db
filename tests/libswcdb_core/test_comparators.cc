/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Comparators.h"
#include <iostream>

namespace Condition = SWC::Condition;

void test(bool vol) {

    const char * a1;
    const char * a2;
    
    // prefix positive
    a1 = "A1";
    a2 = "A1B11111";
    if(!Condition::is_matching(vol, Condition::PF, a1, strlen(a1), a2, strlen(a2)))
       { std::cout << "~PF ERROR vol=" << vol << "!\n"; exit(1);}
    // prefix negative
    a1 = "A1";
    a2 = "A2";
    if(Condition::is_matching(vol, Condition::PF, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "PF ERROR vol=" << vol << "!\n"; exit(1);}

        
    // greater positive
    a1 = "A1";
    a2 = "A1B11111";
    if(!Condition::is_matching(vol, Condition::GT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~GT ERROR vol=" << vol << "!\n"; exit(1);}
    // greater negative
    a1 = "Ab";
    a2 = "Aa";
    if(Condition::is_matching(vol, Condition::GT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "GT ERROR vol=" << vol << "!\n"; exit(1);}
        
    // greater equal positive
    a1 = "A1";
    a2 = "A1";
    if(!Condition::is_matching(vol, Condition::GE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~GE ERROR vol=" << vol << "!\n"; exit(1);}
    // greater equal negative
    a1 = "A2";
    a2 = "A1";
    if(Condition::is_matching(vol, Condition::GE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "GE ERROR vol=" << vol << "!\n"; exit(1);}
        
    // equal positive
    a1 = "A1";
    a2 = "A1";
    if(!Condition::is_matching(vol, Condition::EQ, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~EQ ERROR vol=" << vol << "!\n"; exit(1);}
    // equal negative
    a1 = "A2";
    a2 = "A1";
    if(Condition::is_matching(vol, Condition::EQ, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "EQ ERROR vol=" << vol << "!\n"; exit(1);}
        
    // lower equal positive
    a1 = "A1000";
    a2 = "A0";
    if(!Condition::is_matching(vol, Condition::LE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~LE ERROR vol=" << vol << "!\n"; exit(1);}
    // lower equal negative
    a1 = "A2";
    a2 = "A2222";
    if(Condition::is_matching(vol, Condition::LE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "LE ERROR vol=" << vol << "!\n"; exit(1);}
        
    // lower positive
    a1 = "A1";
    a2 = "A0";
    if(!Condition::is_matching(vol, Condition::LT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~LT ERROR vol=" << vol << "!\n"; exit(1);}
    // lower negative
    a1 = "A0";
    a2 = "A0";
    if(Condition::is_matching(vol, Condition::LT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "LT ERROR vol=" << vol << "!\n"; exit(1);}

    // not equal positive
    a1 = "A1";
    a2 = "A0";
    if(!Condition::is_matching(vol, Condition::NE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~NE ERROR vol=" << vol << "!\n"; exit(1);}
    // not equal negative
    a1 = "A0";
    a2 = "A0";
    if(Condition::is_matching(vol, Condition::NE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "NE ERROR vol=" << vol << "!\n"; exit(1);}

    // regexp positive
    a1 = "^A.*0$";
    a2 = "A5432454350";
    if(!Condition::is_matching(vol, Condition::RE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~RE ERROR vol=" << vol << "!\n"; exit(1);}
    // not regexp negative
    a1 = "^A.*1$";
    a2 = "A5432454350";
    if(Condition::is_matching(vol, Condition::RE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "RE ERROR vol=" << vol << "!\n"; exit(1);}
}

#define LOAD_TEST(_name_, __cond__) \
  ns = SWC::Time::now_ns(); \
    for(uint64_t i=0; i<checks; ++i) SWC_ASSERT(__cond__); \
    took = SWC::Time::now_ns() - ns; \
    std::cout << " took=" << took \
              << " avg=" << took/checks  \
              << " probes=" << probe \
              << " " _name_ << "\n";

void load_check() {
  
  uint64_t ns;
  uint64_t took;
  uint64_t checks = UINT64_MAX;
  const char* buf1 = "abcdefghijk12343455";
  const char* buf2 = "abcdefghijk12343451";
  uint32_t len = 19;
  const uint8_t* ptr1 = (const uint8_t*)buf1;
  const uint8_t* ptr2 = (const uint8_t*)buf2;

  for(int probe=10;probe;--probe) {
    LOAD_TEST(
      "memcmp", 
      memcmp(ptr1, ptr2, 18) == 0
    );
    /* ok, from -O2
    LOAD_TEST(
      "SWC::memcomp", 
      SWC::memcomp(ptr1, ptr2, 18) == 0
    );
    */
    LOAD_TEST(
      "SWC::Condition::is_matching vol=false", 
      SWC::Condition::is_matching(
            false, SWC::Condition::LT, ptr1, len, ptr2, len)
    );

    LOAD_TEST(
      "SWC::Condition::is_matching vol=true", 
      SWC::Condition::is_matching(
            true, SWC::Condition::LT, ptr1, len, ptr2, len)
    );

    LOAD_TEST(
      "SWC::Condition::condition vol=false", 
      SWC::Condition::condition(
        false, ptr1, len, ptr2, len) ==  SWC::Condition::LT
    );

    LOAD_TEST(
      "SWC::Condition::condition vol=true", 
      SWC::Condition::condition(
        true, ptr1, len, ptr2, len) ==  SWC::Condition::LT
    );
  }
}



int main() {
  test(false);
  test(true);

  load_check();

  return 0;
}
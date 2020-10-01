/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include <memory>
#include <vector>
#include <iostream>

#include "swcdb/core/Time.h"
#include "swcdb/core/Serialization.h"

#include "swcdb/core/config/Settings.h"
#include "swcdb/db/Cells/KeyComparator.h"

namespace SWC { 

namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}


SWC_SHOULD_NOT_INLINE
static 
void load_check_compare(const DB::Types::KeySeq key_seq, 
                        int chks, size_t fractions) {
  DB::Cell::Key key1;
  DB::Cell::Key key2;
  for(size_t b=0;b<fractions;++b)
    key1.add(std::to_string(std::rand()));
  key2.copy(key1);

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n)
    SWC_ASSERT(DB::KeySeq::compare(key_seq, key1, key2) == Condition::EQ);
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_compare,      fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << DB::Types::to_string(key_seq) 
            << "\n";
}


SWC_SHOULD_NOT_INLINE
static 
void load_check_compare_max(const DB::Types::KeySeq key_seq, 
                            int chks, size_t fractions) {
  DB::Cell::Key key1;
  DB::Cell::Key key2;
  for(size_t b=0;b<fractions;++b)
    key1.add(std::to_string(std::rand()));
  key2.copy(key1);

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n)
    SWC_ASSERT(
      DB::KeySeq::compare(key_seq, key1, key2, key2.size) == Condition::EQ);
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_compare(max), fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << DB::Types::to_string(key_seq) 
            << "\n";
}

SWC_SHOULD_NOT_INLINE
static 
void load_check_compare_to_vec(const DB::Types::KeySeq key_seq, 
                               int chks, size_t fractions) {
  DB::Cell::Key key1;
  DB::Cell::KeyVec key2;
  std::string f;
  for(size_t b=0;b<fractions;++b) {
    f = std::to_string(std::rand());
    key1.add(f);
    key2.add(f);
  }

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n)
    SWC_ASSERT(DB::KeySeq::compare(key_seq, key1, key2, Condition::GT));
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_compare_vec,  fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << DB::Types::to_string(key_seq) 
            << "\n";
}

SWC_SHOULD_NOT_INLINE
static 
void load_check_align(const DB::Types::KeySeq key_seq, 
                      int chks, size_t fractions) {

  DB::Cell::KeyVec key;
  DB::Cell::KeyVec key1;
  for(size_t b=0;b<fractions;++b)
    key1.add(std::to_string(std::rand()));

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n) {
    DB::KeySeq::align(key_seq, key, key1, Condition::GT);
    SWC_ASSERT(key.size() == fractions);
  }
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_align,        fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << DB::Types::to_string(key_seq)
            << "\n";
}

SWC_SHOULD_NOT_INLINE
static 
void load_check_align_min_max(const DB::Types::KeySeq key_seq, 
                              int chks, size_t fractions) {

  DB::Cell::Key key;
  DB::Cell::KeyVec min;
  DB::Cell::KeyVec max;
  for(size_t b=0;b<fractions;++b)
    key.add(std::to_string(std::rand()));

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n) {
    DB::KeySeq::align(key_seq, key, min, max);
    SWC_ASSERT(min.size() == fractions);
    SWC_ASSERT(max.size() == fractions);
  }

  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_align_min_max,fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << DB::Types::to_string(key_seq)
            << "\n";
}

}


#define LOAD_TEST(_name_, __cond__) \
  ns = SWC::Time::now_ns(); \
  for(uint64_t i=0; i<checks; ++i) SWC_ASSERT(__cond__); \
  took = SWC::Time::now_ns() - ns; \
  std::cout << " took=" << took \
            << " avg=" << took/checks  \
            << " probes=" << probe \
            << " " _name_ << "\n";

SWC_SHOULD_NOT_INLINE
static 
void load_check_condition_base1()  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static 
void load_check_condition_base1() {
  
  uint64_t ns;
  uint64_t took;
  uint64_t checks = UINT64_MAX;
  std::string tmp = std::to_string(std::rand()) + std::to_string(std::rand());
  uint32_t len = tmp.length();
  std::string s1 = tmp.substr(0, len-1) + "2";
  std::string s2 = tmp.substr(0, len-1) + "1";

  const uint8_t* ptr1 = (const uint8_t*)s1.data();
  const uint8_t* ptr2 = (const uint8_t*)s2.data();

  for(int probe=10;probe;--probe) {
    LOAD_TEST(
      "memcmp", 
      memcmp(ptr1, ptr2, s2.size()-1) == 0
    );// base of memcmp for conditions
  }
}

SWC_SHOULD_NOT_INLINE
static 
void load_check_condition_base2()  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static 
void load_check_condition_base2() {
  
  uint64_t ns;
  uint64_t took;
  uint64_t checks = UINT64_MAX;
  std::string tmp = std::to_string(std::rand()) + std::to_string(std::rand());
  uint32_t len = tmp.length();
  std::string s1 = tmp.substr(0, len-1) + "2";
  std::string s2 = tmp.substr(0, len-1) + "1";

  const uint8_t* ptr1 = (const uint8_t*)s1.data();
  const uint8_t* ptr2 = (const uint8_t*)s2.data();

  for(int probe=10;probe;--probe) {
    LOAD_TEST(
      "SWC::DB::KeySeq::condition(LEXIC, ptr1, len, ptr2, len)", 
      SWC::DB::KeySeq::condition(
        SWC::DB::Types::KeySeq::LEXIC,
        ptr1, len, ptr2, len) == SWC::Condition::LT
    );
  }
}

SWC_SHOULD_NOT_INLINE
static 
void load_check_condition_base3()  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static 
void load_check_condition_base3() {
  
  uint64_t ns;
  uint64_t took;
  uint64_t checks = UINT64_MAX;
  std::string tmp = std::to_string(std::rand()) + std::to_string(std::rand());
  uint32_t len = tmp.length();
  std::string s1 = tmp.substr(0, len-1) + "2";
  std::string s2 = tmp.substr(0, len-1) + "1";

  const uint8_t* ptr1 = (const uint8_t*)s1.data();
  const uint8_t* ptr2 = (const uint8_t*)s2.data();

  for(int probe=10;probe;--probe) {
    LOAD_TEST(
      "SWC::DB::KeySeq::condition(VOLUME, ptr1, len, ptr2, len)", 
      SWC::DB::KeySeq::condition(
        SWC::DB::Types::KeySeq::VOLUME,
        ptr1, len, ptr2, len) == SWC::Condition::LT
    );
  }
}


int main() {

  load_check_condition_base1();
  load_check_condition_base2();
  load_check_condition_base3();

  int chks = 1000000;
  std::vector<SWC::DB::Types::KeySeq> sequences = {
    SWC::DB::Types::KeySeq::LEXIC,
    SWC::DB::Types::KeySeq::VOLUME,
    SWC::DB::Types::KeySeq::FC_LEXIC,
    SWC::DB::Types::KeySeq::FC_VOLUME
  };

  for(int fractions=10; fractions<=100; fractions+=10) {
    for(auto key_seq : sequences) {
      SWC::load_check_compare(key_seq, chks, fractions);
      SWC::load_check_compare_to_vec(key_seq, chks, fractions);
      SWC::load_check_compare_max(key_seq, chks, fractions);
      SWC::load_check_align(key_seq, chks, fractions);
      SWC::load_check_align_min_max(key_seq, chks, fractions);
    }
  }


  exit(0);
   
}
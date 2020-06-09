/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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


void load_check_compare(const Types::KeySeq key_seq, int chks, int fractions) {
  DB::Cell::Key key1;
  DB::Cell::Key key2;
  for(auto b=0;b<fractions;++b)
    key1.add(std::to_string(std::rand()));
  key2.copy(key1);

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n)
    SWC_ASSERT(DB::KeySeq::compare(key_seq, key1, key2) == Condition::EQ);
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_compare,      fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << Types::to_string(key_seq) 
            << "\n";
}


void load_check_compare_max(const Types::KeySeq key_seq, int chks, int fractions) {
  DB::Cell::Key key1;
  DB::Cell::Key key2;
  for(auto b=0;b<fractions;++b)
    key1.add(std::to_string(std::rand()));
  key2.copy(key1);

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n)
    SWC_ASSERT(DB::KeySeq::compare(key_seq, key1, key2, key2.size) == Condition::EQ);
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << "load_check_compare(max), fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << Types::to_string(key_seq) 
            << "\n";
}

void load_check_compare_to_vec(const Types::KeySeq key_seq, int chks, int fractions) {
  DB::Cell::Key key1;
  DB::Cell::KeyVec key2;
  std::string f;
  for(auto b=0;b<fractions;++b) {
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
            << " seq=" << Types::to_string(key_seq) 
            << "\n";
}

void load_check_align(const Types::KeySeq key_seq, int chks, int fractions) {

  DB::Cell::KeyVec key;
  DB::Cell::KeyVec key1;
  for(auto b=0;b<fractions;++b)
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
            << " seq=" << Types::to_string(key_seq)
            << "\n";
}

void load_check_align_min_max(const Types::KeySeq key_seq, int chks, int fractions) {

  DB::Cell::Key key;
  DB::Cell::KeyVec min;
  DB::Cell::KeyVec max;
  for(auto b=0;b<fractions;++b)
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
            << " seq=" << Types::to_string(key_seq)
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

void load_check_condition_base() {
  
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
      memcmp(ptr1, ptr2, 18) == 0
    );// base of memcmp for conditions

    LOAD_TEST(
      "SWC::DB::KeySeq::condition(LEXIC, ptr1, len, ptr2, len)", 
      SWC::DB::KeySeq::condition(
        SWC::Types::KeySeq::LEXIC,
        ptr1, len, ptr2, len) == SWC::Condition::LT
    );

    LOAD_TEST(
      "SWC::DB::KeySeq::condition(VOLUME, ptr1, len, ptr2, len)", 
      SWC::DB::KeySeq::condition(
        SWC::Types::KeySeq::VOLUME,
        ptr1, len, ptr2, len) == SWC::Condition::LT
    );

  }
}



int main() {

  load_check_condition_base();

  int chks = 1000000;
  std::vector<SWC::Types::KeySeq> sequences = {
    SWC::Types::KeySeq::LEXIC,
    SWC::Types::KeySeq::VOLUME,
    SWC::Types::KeySeq::FC_LEXIC,
    SWC::Types::KeySeq::FC_VOLUME
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
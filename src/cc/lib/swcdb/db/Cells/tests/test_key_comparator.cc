/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include <memory>
#include <vector>
#include <iostream>
#include <cassert>

#include "swcdb/core/Time.h"
#include "swcdb/core/Serialization.h"

#include "swcdb/core/config/Settings.h"
#include "swcdb/db/Cells/KeyComparator.h"

namespace SWC { 

namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}


void load_check(const Types::KeySeq key_seq, int chks, int fractions) {
  DB::Cell::Key key1;
  DB::Cell::Key key2;
  for(auto b=0;b<fractions;++b)
    key1.add(std::to_string(b+2^60));
  key2.copy(key1);

  auto ts = Time::now_ns();
  for(int n=0; n < chks; ++n)
    assert(DB::KeySeq::compare(key_seq, key1, key2) == Condition::EQ);
  
  uint64_t took = Time::now_ns() - ts;
  std::cout << " fractions=" << fractions 
            << " avg=" << took/chks
            << " took=" << took 
            << " seq=" << Types::to_string(key_seq) 
            << "\n";
}

}

int main() {
  
  std::vector<SWC::Types::KeySeq> sequences = {
    SWC::Types::KeySeq::BITWISE,
    SWC::Types::KeySeq::VOLUME
  };

  int chks = 1000000;

  for(int fractions=0; fractions<=100; ++fractions) {
    for(auto key_seq : sequences) {
      SWC::load_check(key_seq, chks, fractions);
    }
  }

  exit(0);
   
}
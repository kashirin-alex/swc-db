/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include <cassert>
#include "swcdb/core/Exception.h"
#include "swcdb/core/Checksum.h"
#include "swcdb/core/Time.h"

const size_t checks = 10000;
const size_t MB = 64 * 1024 * 1024;
size_t ns;
uint64_t i;


void do_load_fletcher32(const uint8_t* buffer, size_t len) {
  uint32_t tmp_chksum = SWC::Core::fletcher32(buffer, len);
  ns = SWC::Time::now_ns();
  for(i=0; i<checks; ++i) { 
    assert(tmp_chksum == SWC::Core::fletcher32(buffer, len)); 
  }

  ns = SWC::Time::now_ns() - ns; 
  std::cout << " buffer=" << len << " Core::fletcher32 "
            << " took=" << ns << " avg=" << ns/checks
            << " checksum=" << tmp_chksum << "\n";
}

void do_load_checksum_i32(const uint8_t* buffer, size_t len) {
  uint32_t tmp_chksum;
  uint8_t outf[4];
  uint8_t* out;
  ns = SWC::Time::now_ns(); 

  for(i=0; i<checks; ++i) { 
    out = (uint8_t*)&outf;  
    SWC::Core::checksum_i32(buffer, len, &out, tmp_chksum);
    SWC::Core::checksum_i32_chk(tmp_chksum, buffer, len);
  } 

  ns = SWC::Time::now_ns() - ns;
  std::cout << " buffer=" << len << " Core::checksum_i32 "
            << " took=" << ns << " avg=" << ns/checks  << "\n";
}

int main() {

  for(size_t len = 1; len <= MB; len <<= 1) {
    uint8_t* buffer = new uint8_t[len];
    uint8_t* buf = buffer;
    for(size_t fill=0; fill<len; ) {
      for(uint8_t c=0; c<255 && fill<len; ++c, ++fill, ++buf)
        *buf = c;
    }
    do_load_fletcher32(buffer, len);
    do_load_checksum_i32(buffer, len);
    delete[] buffer;
  }

  return 0;
}

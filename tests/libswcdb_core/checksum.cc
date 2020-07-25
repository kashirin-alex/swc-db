/**
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include <cassert>
#include "swcdb/core/Checksum.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Error.h"

const size_t checks = 10;
const size_t MB = 64 * 1024 * 1024;
size_t ns;
uint64_t i;
uint32_t tmp_chksum;

#define LOAD_TEST(_name_, __func__, __sz__) \
  ns = SWC::Time::now_ns(); \
  tmp_chksum = __func__; \
  for(i=1; i<checks; ++i) { assert(__func__ == tmp_chksum); } \
  ns = SWC::Time::now_ns() - ns; \
  std::cout << " buffer=" << __sz__ << " " _name_ \
            << " took=" << ns << " avg=" << ns/checks \
            << " checksum=" << __func__ << "\n";

#define LOAD_TEST_CHECK(_name_, __func__, __sz__) \
  ns = SWC::Time::now_ns(); \
  for(i=1; i<checks; ++i) { __func__;  } \
  ns = SWC::Time::now_ns() - ns; \
  std::cout << " buffer=" << __sz__ << " " _name_ \
            << " took=" << ns << " avg=" << ns/checks  << "\n";


int main() {
  uint8_t outf[4];
  uint8_t* out;

  for(size_t len = 1; len <= MB; len <<= 1) {
    uint8_t* buffer = new uint8_t[len];
    uint8_t* buf = buffer;
    for(size_t fill=0; fill<len; ) {
      for(uint8_t c=0; c<255 && fill<len; ++c, ++fill, ++buf)
        *buf = c;
    }

    LOAD_TEST(
      "fletcher32     ", 
      SWC::fletcher32(buffer, len), 
      len);

    LOAD_TEST_CHECK(
      "checksum_i32   ", 
      out = (uint8_t*)&outf;  
      uint32_t chksum;
      SWC::checksum_i32(buffer, len, &out, chksum);
      SWC::checksum_i32_chk(chksum, buffer, len);
      ,
      len);

    delete buffer;
  }

  return 0;
}

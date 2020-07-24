/**
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include <cassert>
#include "swcdb/core/Checksum.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Error.h"

const size_t checks = 10;
const size_t MB = 1024 * 1024;
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


int main() {

  size_t len;
  for(size_t mb = 1; mb <= 64; ++mb) {
    uint8_t* buffer = new uint8_t[len = mb * MB];
    uint8_t* buf = buffer;
    for(size_t fill=0; fill<len; ) {
      for(uint8_t c=0; c<255 && fill<len; ++c, ++fill, ++buf)
        *buf = c;
    }

    LOAD_TEST("fletcher32", SWC::fletcher32(buffer, len), len);
    LOAD_TEST(" summing32", SWC::summing32(buffer, len), len);

    delete buffer;
  }

  return 0;
}

/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "swcdb/core/Compat.h"
#include "swcdb/core/Logger.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Time.h"

using namespace SWC;
using namespace Serialization;

namespace {

void test_i8() {
  uint8_t buf[1];
  uint8_t input = 0xca;
  *buf = input;
  const uint8_t *p = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding i8",
    SWC_ASSERT(decode_i8(&p, &len) == input);
    SWC_ASSERT(p - buf == 1);
    SWC_ASSERT(!len));
}

void test_i16() {
  uint8_t buf[2], *p = buf;
  uint16_t input = 0xcafe;
  encode_i16(&p, input);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding i16",
    SWC_ASSERT(decode_i16(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 2);
    SWC_ASSERT(!len));

  auto ns = Time::now_ns();
  for(uint16_t n=0; n<UINT16_MAX;++n){
    uint8_t buf[2], *p = buf;
    const uint8_t *p2 = buf;
    size_t len = 2;
    encode_i16(&p, n);
    SWC_TRY("decoding i32",
      SWC_ASSERT(decode_i16(&p2, &len) == n);
      SWC_ASSERT(!len));
  }
  ns = Time::now_ns() - ns;
  std::cout << "i16 took=" << ns
            << " avg=" << double(ns) / UINT16_MAX << "\n";
}

void test_i24() {
  uint8_t buf[3], *p = buf;
  uint24_t input = 0xfebabe;
  encode_i24(&p, 0xfebabe);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding i24",
    SWC_ASSERT(decode_i24(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 3);
    SWC_ASSERT(!len));

  auto ns = Time::now_ns();
  for(uint24_t n=0; n<UINT24_MAX;++n){
    uint8_t buf[3], *p = buf;
    const uint8_t *p2 = buf;
    size_t len = 3;
    encode_i24(&p, n);
    SWC_TRY("decoding i24",
      SWC_ASSERT(decode_i24(&p2, &len) == n);
      SWC_ASSERT(!len));
  }
  ns = Time::now_ns() - ns;
  std::cout << "i24 took=" << ns
            << " avg=" << double(ns) / double(UINT24_MAX) << "\n";
}

void test_i32() {
  uint8_t buf[4], *p = buf;
  uint32_t input = 0xcafebabe;
  encode_i32(&p, 0xcafebabe);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding i32",
    SWC_ASSERT(decode_i32(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 4);
    SWC_ASSERT(!len));

  auto ns = Time::now_ns();
  for(uint32_t n=0; n<UINT32_MAX;++n){
    uint8_t buf[4], *p = buf;
    const uint8_t *p2 = buf;
    size_t len = 4;
    encode_i32(&p, n);
    SWC_TRY("decoding i32",
      SWC_ASSERT(decode_i32(&p2, &len) == n);
      SWC_ASSERT(!len));
  }
  ns = Time::now_ns() - ns;
  std::cout << "i32 took=" << ns
            << " avg=" << double(ns) / double(UINT32_MAX) << "\n";
}

void test_i64() {
  uint8_t buf[8], *p = buf;
  uint64_t input = 0xcafebabeabadbabeull;
  encode_i64(&p, input);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding i64",
    SWC_ASSERT(decode_i64(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 8);
    SWC_ASSERT(!len));

  auto ns = Time::now_ns();
  for(uint64_t n=0; n<UINT64_MAX;++n){
    uint8_t buf[8], *p = buf;
    const uint8_t *p2 = buf;
    size_t len = 8;
    encode_i64(&p, n);
    SWC_TRY("decoding i64",
      SWC_ASSERT(decode_i64(&p2, &len) == n);
      SWC_ASSERT(!len));
  }
  ns = Time::now_ns() - ns;
  std::cout << "i64 took=" << ns
            << " avg=" << double(ns) / double(UINT64_MAX) << "\n";
}

void chk_vi24(uint24_t n) {
  uint8_t buf[4] = {0};
  uint8_t *p = buf;
  const uint8_t *p2 = buf;
  encode_vi24(&p, n);
  SWC_TRY("decoding vint24",
    SWC_ASSERT(decode_vi24(&p2) == n);
    SWC_ASSERT(p2-buf == encoded_length_vi24(n)));
}

const uint32_t MAX_CHECKS = UINT24_MAX;
const uint32_t PROBES = 9;
void test_vi24() {
  {
  uint8_t buf[4], *p = buf;
  uint24_t input = 0xfebabe;
  encode_vi24(&p, 0xfebabe);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding vint24",
    SWC_ASSERT(decode_vi24(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 4);
    SWC_ASSERT(!len));
  }

  uint64_t c = 0;
  auto ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint24_t n=0; n<MAX_CHECKS;++n) {
    chk_vi24(n);
    ++c;
  }
  ns = Time::now_ns() - ns;
  std::cout << "vi24       took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
}

void chk_vi32(uint32_t n) {
  uint8_t buf[5] = {0};
  uint8_t* p = buf;
  const uint8_t *p2 = buf;
  encode_vi32(&p, n);
  SWC_TRY("decoding vint32",
    SWC_ASSERT(decode_vi32(&p2) == n);
    SWC_ASSERT(p2-buf == encoded_length_vi32(n)));
}

void test_vi32() {
  {
  uint8_t buf[5], *p = buf;
  uint32_t input = 0xcafebabe;
  encode_vi32(&p, 0xcafebabe);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding vint32",
    SWC_ASSERT(decode_vi32(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 5);
    SWC_ASSERT(!len));
  }

  uint64_t c = 0;
  auto ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint32_t n=0; n<MAX_CHECKS;++n) {
    chk_vi32(n);
    ++c;
  }
  ns = Time::now_ns() - ns;
  std::cout << "vi32 lower took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
  c = 0;
  ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint32_t n=UINT32_MAX-MAX_CHECKS+1; n <= UINT32_MAX ;++n) {
    chk_vi32(n);
    ++c;
    if(n == UINT32_MAX)
      break;
  }
  ns = Time::now_ns() - ns;
  std::cout << "vi32 upper took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
}

void chk_vi64(uint64_t n) {
  uint8_t buf[10] = {0};
  uint8_t* p = buf;
  const uint8_t *p2 = buf;
  encode_vi64(&p, n);
  SWC_TRY("decoding vint64",
    SWC_ASSERT(decode_vi64(&p2) == n);
    SWC_ASSERT(p2-buf == encoded_length_vi64(n)));
}

void test_vi64() {
  {
  uint8_t buf[10], *p = buf;
  uint64_t input = 0xcafebabeabadbabeull;
  encode_vi64(&p, input);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding vint64",
    SWC_ASSERT(decode_vi64(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 10);
    SWC_ASSERT(!len));
  }

  uint64_t c = 0;
  auto ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint64_t n=0; n<MAX_CHECKS;++n) {
    chk_vi64(n);
    ++c;
  }
  ns = Time::now_ns() - ns;
  std::cout << "vi64 lower took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
  c = 0;
  ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint64_t n=UINT64_MAX-MAX_CHECKS+1; n<=UINT64_MAX;++n) {
    chk_vi64(n);
    ++c;
    if(n == UINT64_MAX)
      break;
  }
  ns = Time::now_ns() - ns;
  std::cout << "vi64 upper took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
}



void chk_fixed_vi24(uint24_t n) {
  uint8_t buf[4] = {0};
  uint8_t *p = buf;
  const uint8_t *p2 = buf;
  encode_fixed_vi24(&p, n);
  SWC_TRY("decoding fixed_vint24",
    SWC_ASSERT(decode_fixed_vi24(&p2) == n);
    SWC_ASSERT(p2-buf == encoded_length_fixed_vi24(n)));
}

void test_fixed_vi24() {
  {
  uint8_t buf[4], *p = buf;
  uint24_t input = 0xfebabe;
  encode_fixed_vi24(&p, 0xfebabe);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding fixed_vint24",
    SWC_ASSERT(decode_fixed_vi24(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 4);
    SWC_ASSERT(!len));
  }

  uint64_t c = 0;
  auto ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint24_t n=0; n<MAX_CHECKS;++n) {
    chk_fixed_vi24(n);
    ++c;
  }
  ns = Time::now_ns() - ns;
  std::cout << "fixed_vi24       took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
}


void chk_fixed_vi32(uint32_t n) {
  uint8_t buf[5] = {0};
  uint8_t* p = buf;
  const uint8_t *p2 = buf;
  encode_fixed_vi32(&p, n);
  SWC_TRY("decoding fixed_vint32",
    SWC_ASSERT(decode_fixed_vi32(&p2) == n);
    SWC_ASSERT(p2-buf == encoded_length_fixed_vi32(n)));
}

void test_fixed_vi32() {
  {
  uint8_t buf[5], *p = buf;
  uint32_t input = 0xcafebabe;
  encode_fixed_vi32(&p, 0xcafebabe);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding fixed_vint32",
    SWC_ASSERT(decode_fixed_vi32(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 5);
    SWC_ASSERT(!len));
  }

  uint64_t c = 0;
  auto ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint32_t n=0; n<MAX_CHECKS;++n) {
    chk_fixed_vi32(n);
    ++c;
  }
  ns = Time::now_ns() - ns;
  std::cout << "fixed_vi32 lower took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
  c = 0;
  ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint32_t n=UINT32_MAX-MAX_CHECKS+1; n <= UINT32_MAX ;++n) {
    chk_fixed_vi32(n);
    ++c;
    if(n == UINT32_MAX)
      break;
  }
  ns = Time::now_ns() - ns;
  std::cout << "fixed_vi32 upper took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
}


void chk_fixed_vi64(uint64_t n) {
  uint8_t buf[9] = {0};
  uint8_t* p = buf;
  const uint8_t *p2 = buf;
  encode_fixed_vi64(&p, n);
  SWC_TRY("decoding fixed_vint64",
    SWC_ASSERT(decode_fixed_vi64(&p2) == n);
    SWC_ASSERT(p2-buf == encoded_length_fixed_vi64(n)));
}

void test_fixed_vi64() {
  {
  uint8_t buf[9], *p = buf;
  uint64_t input = 0xfcafebabeabadbab;
  encode_fixed_vi64(&p, input);
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  SWC_TRY("decoding fixed_vint64",
    SWC_ASSERT(decode_fixed_vi64(&p2, &len) == input);
    SWC_ASSERT(p2 - buf == 9);
    SWC_ASSERT(!len));
  }

  uint64_t c = 0;
  auto ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint64_t n=0; n<MAX_CHECKS;++n) {
    chk_fixed_vi64(n);
    ++c;
  }
  ns = Time::now_ns() - ns;
  std::cout << "fixed_vi64 lower took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
  c = 0;
  ns = Time::now_ns();
  for(uint32_t p=0; p<PROBES;++p)
  for(uint64_t n=UINT64_MAX-MAX_CHECKS+1; n<=UINT64_MAX;++n) {
    chk_fixed_vi64(n);
    ++c;
    if(n == UINT64_MAX)
      break;
  }
  ns = Time::now_ns() - ns;
  std::cout << "fixed_vi64 upper took=" << ns
            << " avg=" << double(ns) / c << " c=" << c << "\n";
}

void test_bytes_string() {
  uint8_t buf[128], *p = buf;
  const char* input = "the quick brown fox jumps over a lazy dog";
  encode_bytes(&p, input, strlen(input));
  const uint8_t *p2 = buf;
  size_t len = sizeof(buf);
  auto s = decode_bytes_string(&p2, &len);
  SWC_TRY("testing bytes_string",
    SWC_ASSERT(!strcmp(s.c_str(), input));
    SWC_ASSERT(p2 - buf == int(encoded_length_bytes(strlen(input))));
    SWC_ASSERT(len == sizeof(buf) - (p2 - buf)));
}

void test_bad_vi24() {
  try {
    uint8_t buf[4] = {0x81, 0xbe, 0xef, 0xca};
    const uint8_t *p = buf;
    size_t len = sizeof(buf);
    decode_vi24(&p, &len);
  }
  catch (const Error::Exception& e) {
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    SWC_ASSERT(e.code() == Error::SERIALIZATION_INPUT_OVERRUN);
  }
}

void test_bad_vi32() {
  try {
    uint8_t buf[5] = {0xde, 0xad, 0xbe, 0xef, 0xca};
    const uint8_t *p = buf;
    size_t len = sizeof(buf);
    decode_vi32(&p, &len);
  }
  catch (const Error::Exception& e) {
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    SWC_ASSERT(e.code() == Error::SERIALIZATION_INPUT_OVERRUN);
  }
}

void test_bad_vi64() {
  try {
    uint8_t buf[14] = {0xab, 0xad, 0xba, 0xbe, 0xde, 0xad, 0xbe, 0xef,
                       0xca, 0xfe, 0xba, 0xbe, 0xbe, 0xef};
    const uint8_t *p = buf;
    size_t len = sizeof(buf);
    decode_vi64(&p, &len);
  }
  catch (const Error::Exception& e) {
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    SWC_ASSERT(e.code() == Error::SERIALIZATION_INPUT_OVERRUN);
  }
}

void test_bad_bytes_string() {
  try {
    uint8_t buf[20] = {0x14, 't', 'h', 'e', ' ', 'b', 'r', 'o', 'w', 'n', ' ',
                       'f', 'o', 'x', ' ', 'j', 'u', 'm', 'p', 's'};
    const uint8_t *p = buf;
    size_t len = sizeof(buf);
    decode_bytes_string(&p, &len);
  }
  catch (const Error::Exception& e) {
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    SWC_ASSERT(e.code() == Error::SERIALIZATION_INPUT_OVERRUN);
  }
}

void test_ser() {
  test_i8();
  test_i16();
  test_i24();
  test_i32();
  test_i64();
  test_vi24();
  test_vi32();
  test_vi64();
  test_fixed_vi24();
  test_fixed_vi32();
  test_fixed_vi64();
  test_bytes_string();

  test_bad_vi24();
  test_bad_vi32();
  test_bad_vi64();
  test_bad_bytes_string();
}

} // local namespace

int main() {
  try {
    test_ser();
  }
  catch (...) {
    SWC_LOG_OUT(LOG_FATAL, SWC_LOG_OSTREAM << SWC_CURRENT_EXCEPTION(""); );
    return 1;
  }
  return 0;
}

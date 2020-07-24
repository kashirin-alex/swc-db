/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_Checksum_h
#define swc_core_Checksum_h

#include "swcdb/core/Compat.h"

namespace SWC {

uint32_t fletcher32(const void* data8, size_t len8);

extern SWC_CAN_INLINE 
uint32_t summing32(const void* data8, size_t len8) {
  uint32_t sum = 0;
  const uint8_t* ptr = (const uint8_t*)data8;
  for(const uint8_t* end = ptr + len8; 
      ptr < end; 
      sum += *ptr, ++ptr
  );
  return sum;
}

extern SWC_CAN_INLINE 
uint32_t checksum32(const void* data8, size_t len8) {
  return fletcher32(data8, len8);
}

bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len);

bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len, 
                      uint32_t offset);

void checksum_i32(const uint8_t* start, size_t len, uint8_t** ptr);

void checksum_i32(const uint8_t* start, size_t len, uint8_t** ptr,
                  uint32_t& checksum);

void checksum_i32(const uint8_t* start, const uint8_t* end, uint8_t** ptr);

void checksum_i32(const uint8_t* start, const uint8_t* end, uint8_t** ptr, 
                  uint32_t& checksum);


} // namespace SWC

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Checksum.cc"
#endif 

#endif /* swc_core_Checksum_h */

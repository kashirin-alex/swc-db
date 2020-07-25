/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_Checksum_h
#define swc_core_Checksum_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Serialization.h"



namespace SWC {

// Fletcher 8-bit implementation (32-bit checksum)
SWC_SHOULD_NOT_INLINE
static 
uint32_t fletcher32(const uint8_t* data, size_t len) {
   uint32_t sum1 = 0, sum2 = 0;
   for(; len; --len, ++data)
      (sum2 += ((sum1 += *data) %= 0xff)) %= 0xff;
   return (sum2 <<= 8) | sum1;
}


extern SWC_CAN_INLINE 
uint32_t checksum32(const uint8_t* data8, size_t len8) {
  return fletcher32((const uint8_t*)data8, len8);
}



extern SWC_CAN_INLINE 
void checksum_i32(const uint8_t* start, size_t len, uint8_t** ptr) {
  Serialization::encode_i32(ptr, checksum32(start, len));
}

extern SWC_CAN_INLINE 
void checksum_i32(const uint8_t* start, const uint8_t* end, uint8_t** ptr) {
  Serialization::encode_i32(ptr, checksum32(start, end-start));
}


extern SWC_CAN_INLINE 
void checksum_i32(const uint8_t* start, size_t len, uint8_t** ptr,
                  uint32_t& checksum) {
  Serialization::encode_i32(ptr, checksum = checksum32(start, len));
}

extern SWC_CAN_INLINE 
void checksum_i32(const uint8_t* start, const uint8_t* end, uint8_t** ptr, 
                  uint32_t& checksum) {
  Serialization::encode_i32(ptr, checksum = checksum32(start, end-start));
}



void checksum_i32_chk_err(uint32_t checksum, uint32_t computed);

extern SWC_CAN_INLINE 
bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len) {
  uint32_t computed = checksum32(base, len);
  if(checksum == computed)
    return true;
  checksum_i32_chk_err(checksum, computed);
  return false;
}

extern SWC_CAN_INLINE 
bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len, 
                      uint32_t offset) {
  memset((void*)(base + offset), 0, 4);
  return checksum_i32_chk(checksum, base, len);
}



} // namespace SWC

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Checksum.cc"
#endif 

#endif /* swc_core_Checksum_h */

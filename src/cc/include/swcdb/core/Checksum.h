/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Checksum_h
#define swcdb_core_Checksum_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Serialization.h"



namespace SWC { namespace Core {



// Fletcher 8-bit by 16-bit implementation (32-bit checksum)
static 
uint32_t fletcher32(const void *data8, size_t len) noexcept
  __attribute__((optimize("-O3")));

SWC_SHOULD_NOT_INLINE
static 
uint32_t fletcher32(const void *data8, size_t len) noexcept {
  uint32_t c0 = 0, c1 = 0;

  const uint16_t* data = (const uint16_t *)data8;
  bool align = len & 0x01;
  len >>= 1;

  for(uint16_t i; len >= 360; len -= 360) {
    for(i = 360; i; --i, ++data)
      c1 += (c0 += *data);
    c0 %= 65535;
    c1 %= 65535;
  }
  
  if(align || len) {
    for(;len; ++data, --len)
      c1 += (c0 += *data);
    if(align)
      c1 += (c0 += *(const uint8_t*)data);
    c0 %= 65535;
    c1 %= 65535;
  }
  return (c1 << 16 | c0);
}


uint32_t checksum32(const uint8_t* data8, size_t len8) noexcept
  __attribute__((optimize("-O3")));

extern SWC_CAN_INLINE 
uint32_t checksum32(const uint8_t* data8, size_t len8) noexcept {
  return fletcher32(data8, len8);
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



}} // namespace SWC::Core


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Checksum.cc"
#endif 

#endif // swcdb_core_Checksum_h

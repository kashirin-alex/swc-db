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
uint32_t fletcher32(const uint8_t* data, size_t len) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static
uint32_t fletcher32(const uint8_t* data, size_t len) noexcept {
  uint32_t c0 = 0, c1 = 0;
  for(uint16_t i; len >= 720; len -= 720) {
    for(i = 360; i; --i, ++data) {
      c0 += uint16_t(*data) << 8;
      c1 += (c0 += *++data);
    }
    c0 %= 65535;
    c1 %= 65535;
  }

  if(len) {
    for(;len > 1; ++data, len-=2) {
      c0 += uint16_t(*data) << 8;
      c1 += (c0 += *++data);
    }
    if(len)
      c1 += (c0 += *data);
    c0 %= 65535;
    c1 %= 65535;
  }
  return (c1 << 16 | c0);
}


uint32_t checksum32(const uint8_t* data8, size_t len8) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
uint32_t checksum32(const uint8_t* data8, size_t len8) noexcept {
  return fletcher32(data8, len8);
}



extern SWC_CAN_INLINE
void checksum_i32(const uint8_t* start, size_t len,
                  uint8_t** ptr) noexcept {
  Serialization::encode_i32(ptr, checksum32(start, len));
}

extern SWC_CAN_INLINE
void checksum_i32(const uint8_t* start, const uint8_t* end,
                  uint8_t** ptr) noexcept {
  Serialization::encode_i32(ptr, checksum32(start, end-start));
}


extern SWC_CAN_INLINE
void checksum_i32(const uint8_t* start, size_t len, uint8_t** ptr,
                  uint32_t& checksum) noexcept {
  Serialization::encode_i32(ptr, checksum = checksum32(start, len));
}

extern SWC_CAN_INLINE
void checksum_i32(const uint8_t* start, const uint8_t* end, uint8_t** ptr,
                  uint32_t& checksum) noexcept {
  Serialization::encode_i32(ptr, checksum = checksum32(start, end-start));
}



bool checksum_i32_log_chk(uint32_t checksum, const uint8_t* base,
                          uint32_t len);



extern SWC_CAN_INLINE
bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len) {
  return checksum == checksum32(base, len);
}

extern SWC_CAN_INLINE
bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len,
                      uint32_t* computed) {
  return checksum == (*computed = checksum32(base, len));
}

extern SWC_CAN_INLINE
bool checksum_i32_chk(uint32_t checksum, const uint8_t* base, uint32_t len,
                      uint32_t offset) {
  memset(const_cast<uint8_t*>(base + offset), 0, 4);
  return checksum_i32_chk(checksum, base, len);
}



}} // namespace SWC::Core


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Checksum.cc"
#endif

#endif // swcdb_core_Checksum_h

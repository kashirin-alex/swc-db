/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#ifndef swcdb_core_Serialization_h
#define swcdb_core_Serialization_h


#include "swcdb/core/Exception.h"



namespace SWC {


/**
 * @brief The SWC-DB Serializations C++ namespace 'SWC::Serialization'
 *
 * \ingroup Core
 */
namespace Serialization {


#define SWC_THROW_OVERRUN(_s_) \
  SWC_THROWF(Error::SERIALIZATION_INPUT_OVERRUN, "Error decoding %s", _s_)

#define SWC_THROW_UNPOSSIBLE(_s_) \
  SWC_THROWF(Error::UNPOSSIBLE, "%s", _s_)


const uint64_t MAX_V1B = 0x7f;
const uint64_t MAX_V2B = 0x3fff;
const uint64_t MAX_V3B = 0x1fffff;
const uint64_t MAX_V4B = 0x0fffffff;
const uint64_t MAX_V5B = 0x07ffffffff;
const uint64_t MAX_V6B = 0x03ffffffffff;
const uint64_t MAX_V7B = 0x01ffffffffffff;
const uint64_t MAX_V8B = 0x00ffffffffffffff;
const uint64_t MAX_V9B = 0x7fffffffffffffff;

const uint8_t MAX_LEN_VINT24 = 4;
const uint8_t MAX_LEN_VINT32 = 5;
const uint8_t MAX_LEN_VINT64 = 10;


extern SWC_CAN_INLINE
void memcopy(uint8_t* dest, const uint8_t** bufp, size_t len) {
  memcpy(dest, *bufp, len);
  *bufp += len;

  //for(; len; --len, ++dest, ++*bufp) *dest = **bufp;
}

extern SWC_CAN_INLINE
void memcopy(uint8_t** bufp, const uint8_t* src, size_t len) {
  memcpy(*bufp, src, len);
  *bufp += len;

  //for(; len; --len, ++*bufp, ++src) **bufp = *src;
}

extern SWC_CAN_INLINE
void decode_needed_one(size_t* remainp) {
  if(!*remainp)
    SWC_THROW(Error::SERIALIZATION_INPUT_OVERRUN,
              "Zero byte remain need 1 byte");
  --*remainp;
}

extern SWC_CAN_INLINE
void decode_needed(size_t* remainp, size_t len) {
  if(*remainp < len)
      SWC_THROWF(Error::SERIALIZATION_INPUT_OVERRUN,
                "Need %lu bytes but only %lu remain", len, *remainp);
  *remainp -= len;
}

extern SWC_CAN_INLINE
void encode_i8(uint8_t** bufp, uint8_t val) {
  **bufp = val;
  ++*bufp;
}

extern SWC_CAN_INLINE
uint8_t decode_i8(const uint8_t** bufp, size_t* remainp) {
  decode_needed_one(remainp);
  return *(*bufp)++;
}

extern SWC_CAN_INLINE
uint8_t decode_byte(const uint8_t** bufp, size_t* remainp) {
  return decode_i8(bufp, remainp);
}

extern SWC_CAN_INLINE
void encode_bool(uint8_t** bufp, bool bval) {
  encode_i8(bufp, bval);
}

extern SWC_CAN_INLINE
bool decode_bool(const uint8_t** bufp, size_t* remainp) {
  return decode_i8(bufp, remainp);
}

extern SWC_CAN_INLINE
void encode_i16(uint8_t** bufp , uint16_t val) {
  memcopy(bufp, (const uint8_t*)&val, 2);
}

extern SWC_CAN_INLINE
uint16_t decode_i16(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 2);
  uint16_t val;
  memcopy((uint8_t*)&val, bufp, 2);
  return val;
}

extern SWC_CAN_INLINE
void encode_i24(uint8_t** bufp , uint24_t val) {
  memcopy(bufp, (const uint8_t*)&val, 3);
}

extern SWC_CAN_INLINE
uint24_t decode_i24(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 3);
  uint24_t val;
  memcopy((uint8_t*)&val, bufp, 3);
  return val;
}

extern SWC_CAN_INLINE
void encode_i32(uint8_t** bufp, uint32_t val) {
  memcopy(bufp, (const uint8_t*)&val, 4);
}

extern SWC_CAN_INLINE
uint32_t decode_i32(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 4);
  uint32_t val;
  memcopy((uint8_t*)&val, bufp, 4);
  return val;
}

extern SWC_CAN_INLINE
void encode_i64(uint8_t** bufp, uint64_t val) {
  memcopy(bufp, (const uint8_t*)&val, 8);
}

extern SWC_CAN_INLINE
uint64_t decode_i64(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 8);
  uint64_t val;
  memcopy((uint8_t*)&val, bufp, 8);
  return val;
}

#define SWC_ENCODE_VI_0(_p_, _v_) \
  if(_v_ <= MAX_V1B) { **_p_ = _v_; ++*_p_; return; }

#define SWC_ENCODE_VI_1(_p_, _v_) \
  **_p_ = _v_ | 0x80; ++*_p_; _v_ >>= 7; \
  SWC_ENCODE_VI_0(_p_, _v_);

#define SWC_ENCODE_VI(_p_, _v_, _shifts_) \
  SWC_ENCODE_VI_0(_p_, _v_); \
  for(uint8_t n=0; n < _shifts_; ++n) { SWC_ENCODE_VI_1(_p_, _v_); } \
  SWC_THROW_UNPOSSIBLE("breached encoding length");


#define SWC_DECODE_NEED_BYTE(_r_) \
  if(!*_r_) SWC_THROW(Error::SERIALIZATION_INPUT_OVERRUN, \
                      "Zero byte remain need 1 byte"); \
  --*_r_;

#define SWC_DECODE_VI_(_v_, _tmp_, _p_, _shift_) \
  _v_ |= (_tmp_ = **_p_ & 0x7f) <<= _shift_; \
  _tmp_ = **_p_ & 0x80; ++*_p_; if(!tmp) return _v_;

#define SWC_DECODE_VI_1(_v_, _tmp_, _p_, _r_, _shift_) \
  SWC_DECODE_NEED_BYTE(_r_);  \
  SWC_DECODE_VI_(_v_, _tmp_, _p_, _shift_);

#define SWC_DECODE_VI(_t_, _p_, _r_, _bits_, _name_) \
  _t_ n = 0; _t_ tmp; for(uint8_t shift=0; *_r_ ; shift+=7) { \
    --*_r_; SWC_DECODE_VI_(n, tmp, _p_, shift); \
    if(shift == _bits_) SWC_THROW_OVERRUN(_name_); \
  } \
  SWC_THROW(Error::SERIALIZATION_INPUT_OVERRUN, \
            "Zero byte remain need 1 byte");


extern SWC_CAN_INLINE
uint8_t encoded_length_vi24(uint24_t val) {
  return
  (val <= MAX_V1B ? 1 :
   (val <= MAX_V2B ? 2 :
    (val <= MAX_V3B ? 3 : 4)));
}

extern SWC_CAN_INLINE
void encode_vi24(uint8_t** bufp, uint24_t val) {
  //SWC_ENCODE_VI(bufp, val, 3);
  SWC_ENCODE_VI_0(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_THROW_UNPOSSIBLE("breached encoding length");
}

extern SWC_CAN_INLINE
uint24_t decode_vi24(const uint8_t** bufp, size_t* remainp) {
  //SWC_DECODE_VI(uint24_t, bufp, remainp, 21, "vint24");
  uint24_t n = 0; uint24_t tmp;
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 0);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 7);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 14);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 21);
  SWC_THROW_OVERRUN("vint24");
}
extern SWC_CAN_INLINE
uint24_t decode_vi24(const uint8_t** bufp) {
  size_t remain = 4;
  return decode_vi24(bufp, &remain);
}


extern SWC_CAN_INLINE
uint8_t encoded_length_vi32(uint32_t val) {
  return
  (val <= MAX_V1B ? 1 :
   (val <= MAX_V2B ? 2 :
    (val <= MAX_V3B ? 3 :
     (val <= MAX_V4B ? 4 : 5))));
}

extern SWC_CAN_INLINE
void encode_vi32(uint8_t** bufp, uint32_t val) {
  //SWC_ENCODE_VI(bufp, val, 4);
  SWC_ENCODE_VI_0(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_THROW_UNPOSSIBLE("breached encoding length");
}

extern SWC_CAN_INLINE
uint32_t decode_vi32(const uint8_t** bufp, size_t* remainp) {
  //SWC_DECODE_VI(uint32_t, bufp, remainp, 28, "vint32");
  uint32_t n = 0; uint32_t tmp;
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 0);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 7);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 14);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 21);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 28);
  SWC_THROW_OVERRUN("vint32");
}

extern SWC_CAN_INLINE
uint32_t decode_vi32(const uint8_t** bufp) {
  size_t remain = 5;
  return decode_vi32(bufp, &remain);
}

extern SWC_CAN_INLINE
uint8_t encoded_length_vi64(uint64_t val) {
  return
  (val <= MAX_V1B ? 1 :
   (val <= MAX_V2B ? 2 :
    (val <= MAX_V3B ? 3 :
     (val <= MAX_V4B ? 4 :
      (val <= MAX_V5B ? 5 :
       (val <= MAX_V6B ? 6 :
        (val <= MAX_V7B ? 7 :
         (val <= MAX_V8B ? 8 :
          (val <= MAX_V9B ? 9 : 10)))))))));
}

extern SWC_CAN_INLINE
void encode_vi64(uint8_t** bufp, uint64_t val) {
  //SWC_ENCODE_VI(bufp, val, 9);
  SWC_ENCODE_VI_0(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_ENCODE_VI_1(bufp, val);
  SWC_THROW_UNPOSSIBLE("breached encoding length");
}

extern SWC_CAN_INLINE
uint64_t decode_vi64(const uint8_t** bufp, size_t* remainp) {
  //SWC_DECODE_VI(uint64_t, bufp, remainp, 63, "vint64");
  uint64_t n = 0; uint64_t tmp;
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 0);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 7);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 14);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 21);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 28);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 35);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 42);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 49);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 56);
  SWC_DECODE_VI_1(n, tmp, bufp, remainp, 63);
  SWC_THROW_OVERRUN("vint64");
}

extern SWC_CAN_INLINE
uint64_t decode_vi64(const uint8_t** bufp) {
  size_t remain = 10;
  return decode_vi64(bufp, &remain);
}


template<uint8_t BITS, uint8_t SZ, typename T>
extern SWC_CAN_INLINE
void encode_fixed_vi(uint8_t** bufp, Core::BitFieldInt<T, BITS> val) {
  **bufp = SZ;
  memcpy(++*bufp, (const uint8_t*)&val, SZ);
  *bufp += SZ;
}

template<uint8_t BITS, uint8_t SZ, typename T>
extern SWC_CAN_INLINE
T decode_fixed_vi(const uint8_t** bufp) {
  Core::BitFieldInt<T, BITS> val;
  memcpy((uint8_t*)&val, *bufp, SZ);
  *bufp += SZ;
  return val;
}


extern SWC_CAN_INLINE
uint8_t encoded_length_fixed_vi24(uint24_t val) {
  switch((uint32_t)val) {
    case 0x00       ... 0xFB:
      return 1;
    case 0xFB   + 1 ... 0xFF:
      return 2;
    case 0xFF   + 1 ... 0xFFFF:
      return 3;
    default:
      return 4;
  }
}

extern SWC_CAN_INLINE
void encode_fixed_vi24(uint8_t** bufp, uint24_t val) {
  switch((uint32_t)val) {
    case 0x00       ... 0xFB:
     return encode_i8(bufp, val + 4);
    case 0xFB   + 1 ... 0xFF:
      return encode_fixed_vi<8,  1, uint32_t>(bufp, val);
    case 0xFF   + 1 ... 0xFFFF:
      return encode_fixed_vi<16, 2, uint32_t>(bufp, val);
    default:
      return encode_fixed_vi<24, 3, uint32_t>(bufp, val);
  }
}

extern SWC_CAN_INLINE
uint24_t decode_fixed_vi24(const uint8_t** bufp, size_t* remainp) {
  uint8_t b = decode_i8(bufp, remainp);
  if(b > 3)
    return b - 4;
  decode_needed(remainp, b);
  switch(b) {
    case 1:
      return decode_fixed_vi<8,  1, uint32_t>(bufp);
    case 2: 
      return decode_fixed_vi<16, 2, uint32_t>(bufp);
    default: 
      return decode_fixed_vi<24, 3, uint32_t>(bufp);
  }
}

extern SWC_CAN_INLINE
uint24_t decode_fixed_vi24(const uint8_t** bufp) {
  size_t remain = 4;
  return decode_fixed_vi24(bufp, &remain);
}


extern SWC_CAN_INLINE
uint8_t encoded_length_fixed_vi32(uint32_t val) {
  switch(val) {
    case 0x00       ... 0xFA:
      return 1;
    case 0xFA   + 1 ... 0xFF:
      return 2;
    case 0xFF   + 1 ... 0xFFFF:
      return 3;
    case 0xFFFF + 1 ... 0xFFFFFF:
      return 4;
    default:
      return 5;
  }
}

extern SWC_CAN_INLINE
void encode_fixed_vi32(uint8_t** bufp, uint32_t val) {
  switch(val) {
    case 0x00       ... 0xFA:
      return encode_i8(bufp, val + 5);
    case 0xFA + 1   ... 0xFF: 
      return encode_fixed_vi<8,  1, uint32_t>(bufp, val);
    case 0xFF + 1   ... 0xFFFF:
      return encode_fixed_vi<16, 2, uint32_t>(bufp, val);
    case 0xFFFF + 1 ... 0xFFFFFF:
      return encode_fixed_vi<24, 3, uint32_t>(bufp, val);
    default:
      return encode_fixed_vi<32, 4, uint32_t>(bufp, val);
  }
}

extern SWC_CAN_INLINE
uint32_t decode_fixed_vi32(const uint8_t** bufp, size_t* remainp) {
  uint8_t b = decode_i8(bufp, remainp);
  if(b > 4)
    return b - 5;
  decode_needed(remainp, b);
  switch(b) {
    case 1:
      return decode_fixed_vi<8,  1, uint32_t>(bufp);
    case 2:
      return decode_fixed_vi<16, 2, uint32_t>(bufp);
    case 3:
      return decode_fixed_vi<24, 3, uint32_t>(bufp);
    default:
      return decode_fixed_vi<32, 4, uint32_t>(bufp);
  }
}

extern SWC_CAN_INLINE
uint32_t decode_fixed_vi32(const uint8_t** bufp) {
  size_t remain = 5;
  return decode_fixed_vi32(bufp, &remain);
}


extern SWC_CAN_INLINE
uint8_t encoded_length_fixed_vi64(uint64_t val) {
  switch(val) {
    case 0x00                 ... 0xF6:
      return 1;
    case 0xF6             + 1 ... 0xFF:
      return 2;
    case 0xFF             + 1 ... 0xFFFF:
      return 3;
    case 0xFFFF           + 1 ... 0xFFFFFF:
      return 4;
    case 0xFFFFFF         + 1 ... 0xFFFFFFFF:
      return 5;
    default:  
      break;
  }
  if(val < 0xFFFFFFFFFF)
    return 6;
  if(val < 0xFFFFFFFFFFFF)
    return 7;
  if(val < 0xFFFFFFFFFFFFFF)
    return 8;
  return 9;
}

extern SWC_CAN_INLINE
void encode_fixed_vi64(uint8_t** bufp, uint64_t val) {
  switch(val) {
    case 0x00                 ... 0xF6:
      return encode_i8(bufp, val + 9);
    case 0xF6             + 1 ... 0xFF: 
      return encode_fixed_vi<8,  1, uint64_t>(bufp, val);
    case 0xFF             + 1 ... 0xFFFF:
      return encode_fixed_vi<16, 2, uint64_t>(bufp, val);
    case 0xFFFF           + 1 ... 0xFFFFFF:
      return encode_fixed_vi<24, 3, uint64_t>(bufp, val); 
    case 0xFFFFFF         + 1 ... 0xFFFFFFFF:
      return encode_fixed_vi<32, 4, uint64_t>(bufp, val); 
    default: 
      break;
  }
  if(val < 0xFFFFFFFFFF)
    return encode_fixed_vi<40, 5, uint64_t>(bufp, val);
  if(val < 0xFFFFFFFFFFFF)
    return encode_fixed_vi<48, 6, uint64_t>(bufp, val);
  if(val < 0xFFFFFFFFFFFFFF)
    return encode_fixed_vi<56, 7, uint64_t>(bufp, val);
  return encode_fixed_vi<64, 8, uint64_t>(bufp, val);
}

extern SWC_CAN_INLINE
uint64_t decode_fixed_vi64(const uint8_t** bufp, size_t* remainp) {
  uint8_t b = decode_i8(bufp, remainp);
  if(b > 8)
    return b - 9;
  decode_needed(remainp, b);
  switch(b) {
    case 1:
      return decode_fixed_vi<8,  1, uint64_t>(bufp);
    case 2:
      return decode_fixed_vi<16, 2, uint64_t>(bufp);
    case 3:
      return decode_fixed_vi<24, 3, uint64_t>(bufp);
    case 4:
      return decode_fixed_vi<32, 4, uint64_t>(bufp);
    case 5:
      return decode_fixed_vi<40, 5, uint64_t>(bufp);
    case 6:
      return decode_fixed_vi<48, 6, uint64_t>(bufp);
    case 7:
      return decode_fixed_vi<56, 7, uint64_t>(bufp);
    default:
      return decode_fixed_vi<64, 8, uint64_t>(bufp);
  }
}

extern SWC_CAN_INLINE
uint64_t decode_fixed_vi64(const uint8_t** bufp) {
  size_t remain = 9;
  return decode_fixed_vi64(bufp, &remain);
}



extern SWC_CAN_INLINE
uint8_t encoded_length_double() {
  return sizeof(long double);
}

extern SWC_CAN_INLINE
void encode_double(uint8_t** bufp, long double val) {
  memcopy(bufp, (const uint8_t*)&val, encoded_length_double());
}

extern SWC_CAN_INLINE
long double decode_double(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, sizeof(long double));
  long double v;
  memcopy((uint8_t*)&v, bufp, encoded_length_double());
  return v;
}



extern SWC_CAN_INLINE
size_t encoded_length_bytes(size_t len) {
  return encoded_length_vi64(len) + len;
}

extern SWC_CAN_INLINE
void encode_bytes(uint8_t** bufp, const void* data, size_t len) {
  encode_vi64(bufp, len);
  memcopy(bufp, (const uint8_t*)data, len);
}

extern SWC_CAN_INLINE
const uint8_t* decode_bytes(const uint8_t** bufp, size_t* remainp,
                            size_t* lenp) {
  *lenp = decode_vi64(bufp, remainp);
  decode_needed(remainp, *lenp);
  const uint8_t* out = *bufp;
  *bufp += *lenp;
  return out;
}

extern SWC_CAN_INLINE
std::string decode_bytes_string(const uint8_t** bufp, size_t* remainp) {
  size_t len;
  const char* s = (const char*)decode_bytes(bufp, remainp, &len);
  return std::string(s, len);
}



extern SWC_CAN_INLINE
void encode_bytes_fixed(uint8_t** bufp, const void* data,
                        uint32_t len) {
  memcopy(bufp, (const uint8_t*)data, len);
}

extern SWC_CAN_INLINE
const uint8_t* decode_bytes_fixed(const uint8_t** bufp, size_t* remainp,
                                  uint32_t len) {
  decode_needed(remainp, len);
  const uint8_t* out = *bufp;
  *bufp += len;
  return out;
}



}}

#endif // swcdb_core_Serialization_h

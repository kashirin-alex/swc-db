/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/Serialization.h"


//#ifdef SWC_IMPL_SOURCE
# define SWC_CAN_INLINE  \
  __attribute__((__always_inline__, __artificial__)) \
  extern inline
//#else 
//# define SWC_CAN_INLINE 
//#endif

#define SWC_THROW_OVERRUN(_s_) \
  SWC_THROWF(Error::SERIALIZATION_INPUT_OVERRUN, "Error decoding %s", _s_)
#define SWC_THROW_UNPOSSIBLE(_s_) \
  SWC_THROWF(Error::UNPOSSIBLE, "%s", _s_)

extern "C" { }

namespace SWC { namespace Serialization {

const uint8_t  MAX_V1B= 0x7f;
const uint16_t MAX_V2B= 0x3fff;
const uint32_t MAX_V3B= 0x1fffff;
const uint32_t MAX_V4B= 0xfffffff;
const uint64_t MAX_V5B= 0x7ffffffffull;
const uint64_t MAX_V6B= 0x3ffffffffffull;
const uint64_t MAX_V7B= 0x1ffffffffffffull;
const uint64_t MAX_V8B= 0xffffffffffffffull;
const uint64_t MAX_V9B= 0x7fffffffffffffffull;

const uint8_t MAX_LEN_VINT24 = 4;
const uint8_t MAX_LEN_VINT32 = 5;
const uint8_t MAX_LEN_VINT64 = 10;


SWC_CAN_INLINE 
void memcopy(uint8_t* dest, const uint8_t** bufp, size_t len) {
  memcpy(dest, *bufp, len);
  *bufp += len;
  //while (len--)
  //  *dest++ = *(*bufp)++;
}

SWC_CAN_INLINE 
void memcopy(uint8_t** bufp, const uint8_t* src, size_t len) {  
  memcpy(*bufp, src, len);
  *bufp += len;
  //while (len--)
  //  *(*bufp)++ = *src++;
}

SWC_CAN_INLINE 
void decode_needed_one(size_t* remainp) {
  if(!*remainp)
    SWC_THROWF(Error::SERIALIZATION_INPUT_OVERRUN, 
              "Need 1 byte but only %llu remain", *remainp);
  --*remainp;
}

SWC_CAN_INLINE 
void decode_needed(size_t* remainp, size_t len) {
  if(*remainp < len)
      SWC_THROWF(Error::SERIALIZATION_INPUT_OVERRUN, 
                "Need %llu bytes but only %llu remain", len, *remainp);
  *remainp -= len;          
}

SWC_CAN_INLINE 
void encode_i8(uint8_t** bufp, uint8_t val) {
  *(*bufp)++ = val;
}

SWC_CAN_INLINE 
uint8_t decode_i8(const uint8_t** bufp, size_t* remainp) {
  decode_needed_one(remainp);
  return *(*bufp)++;
}

SWC_CAN_INLINE 
uint8_t decode_byte(const uint8_t** bufp, size_t* remainp) {
  return decode_i8(bufp, remainp);
}

SWC_CAN_INLINE 
void encode_bool(uint8_t** bufp, bool bval) {
  *(*bufp)++ = bval ? 1 : 0;
}

SWC_CAN_INLINE 
bool decode_bool(const uint8_t** bufp, size_t* remainp) {
  return decode_i8(bufp, remainp);
}

SWC_CAN_INLINE 
void encode_i16(uint8_t** bufp , uint16_t val) {
  memcopy(bufp, (const uint8_t*)&val, 2);
}

SWC_CAN_INLINE 
uint16_t decode_i16(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 2);
  uint16_t val;
  memcopy((uint8_t*)&val, bufp, 2);
  return val;
}

SWC_CAN_INLINE 
void encode_i24(uint8_t** bufp , uint24_t val) {
  memcopy(bufp, (const uint8_t*)&val, 3);
}

SWC_CAN_INLINE 
uint24_t decode_i24(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 3);
  uint24_t val;
  memcopy((uint8_t*)&val, bufp, 3);
  return val;
}

SWC_CAN_INLINE 
void encode_i32(uint8_t** bufp, uint32_t val) {
  memcopy(bufp, (const uint8_t*)&val, 4);
}

SWC_CAN_INLINE 
uint32_t decode_i32(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 4);
  uint32_t val;
  memcopy((uint8_t*)&val, bufp, 4);
  return val;
}

SWC_CAN_INLINE 
void encode_i64(uint8_t** bufp, uint64_t val) {
  memcopy(bufp, (const uint8_t*)&val, 8);
}

SWC_CAN_INLINE 
uint64_t decode_i64(const uint8_t** bufp, size_t* remainp) {
  decode_needed(remainp, 8);
  uint64_t val;
  memcopy((uint8_t*)&val, bufp, 8);
  return val;
}


SWC_CAN_INLINE 
int encoded_length_vi24(uint24_t val) {
  return 
  (val <= MAX_V1B ? 1 : 
   (val <= MAX_V2B ? 2 : 
    (val <= MAX_V3B ? 3 : 4)));
}
SWC_CAN_INLINE 
bool _encode_vi0(uint8_t** bufp, const uint24_t& val) {
  if(val > MAX_V1B) 
    return true;
  *(*bufp)++ = val;
  return false;
}
SWC_CAN_INLINE 
bool _encode_vi(uint8_t** bufp, uint24_t& val) {
  *(*bufp)++ = val | 0x80;
  return _encode_vi0(bufp, val >>= 7);
}
SWC_CAN_INLINE 
void encode_vi24(uint8_t** bufp, uint24_t val) {
  if(_encode_vi0(bufp, val) && 
     _encode_vi(bufp, val) && 
     _encode_vi(bufp, val) && 
     _encode_vi(bufp, val)) {
    SWC_THROW_UNPOSSIBLE("reach here encoding vint24");
  }
}
SWC_CAN_INLINE 
uint24_t decode_vi24(const uint8_t** bufp, size_t* remainp) {
  uint24_t n = 0;
  uint24_t tmp;
  for(uint8_t shift=0; ; shift+=7) {
    decode_needed_one(remainp);
    n |= (tmp = **bufp & 0x7f) << shift;
    if(!(*(*bufp)++ & 0x80))
      return n;
    if(shift == 28)
      SWC_THROW_OVERRUN("vint24");
  }
}
SWC_CAN_INLINE 
uint24_t decode_vi24(const uint8_t** bufp) {
  size_t remain = 4;
  return decode_vi24(bufp, &remain);
}


SWC_CAN_INLINE 
int encoded_length_vi32(uint32_t val) {
  return 
  (val <= MAX_V1B ? 1 : 
   (val <= MAX_V2B ? 2 : 
    (val <= MAX_V3B ? 3 : 
     (val <= MAX_V4B ? 4 : 5))));
}

SWC_CAN_INLINE 
bool _encode_vi0(uint8_t** bufp, const uint32_t& val) {
  if(val > MAX_V1B) 
    return true;
  *(*bufp)++ = val;
  return false;
}
SWC_CAN_INLINE 
bool _encode_vi(uint8_t** bufp, uint32_t& val) {
  *(*bufp)++ = val | 0x80;
  return _encode_vi0(bufp, val >>= 7);
}
SWC_CAN_INLINE 
void encode_vi32(uint8_t** bufp, uint32_t val) {
  if(_encode_vi0(bufp, val) && 
     _encode_vi(bufp, val) && _encode_vi(bufp, val) && 
     _encode_vi(bufp, val) && _encode_vi(bufp, val)) {
    SWC_THROW_UNPOSSIBLE("reach here encoding vint32");
  }
}

SWC_CAN_INLINE 
uint32_t decode_vi32(const uint8_t** bufp, size_t* remainp) {
  uint32_t n = 0;
  for(uint8_t shift=0; ; shift+=7) {
    decode_needed_one(remainp);
    n |= (uint64_t)(**bufp & 0x7f) << shift;
    if(!(*(*bufp)++ & 0x80))
      return n;
    if(shift == 35)
      SWC_THROW_OVERRUN("vint32");
  }
}

SWC_CAN_INLINE 
uint32_t decode_vi32(const uint8_t** bufp) {
  size_t remain = 5;
  return decode_vi32(bufp, &remain);
}

SWC_CAN_INLINE 
int encoded_length_vi64(uint64_t val) {
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

SWC_CAN_INLINE 
bool _encode_vi0(uint8_t** bufp, const uint64_t& val) {
  if(val > MAX_V1B) 
    return true;
  *(*bufp)++ = (uint8_t)val;
  return false;
}
SWC_CAN_INLINE 
bool _encode_vi(uint8_t** bufp, uint64_t& val) {
  *(*bufp)++ = val | 0x80;
  return _encode_vi0(bufp, val >>= 7);
}
SWC_CAN_INLINE 
void encode_vi64(uint8_t** bufp, uint64_t val) {
  if(_encode_vi0(bufp, val) && 
     _encode_vi(bufp, val) && _encode_vi(bufp, val) && 
     _encode_vi(bufp, val) && _encode_vi(bufp, val) && 
     _encode_vi(bufp, val) && _encode_vi(bufp, val) && 
     _encode_vi(bufp, val) && _encode_vi(bufp, val) &&
     _encode_vi(bufp, val)) {
    SWC_THROW_UNPOSSIBLE("reach here encoding vint64");
  }
}
/* 10%+ perf degredation
SWC_CAN_INLINE 
void encode_vi64(uint8_t** bufp, uint64_t val) {
  for(uint8_t n=0; val > MAX_V1B; ++n, val >>= 7) {
    *(*bufp)++ = (uint8_t)(val | 0x80);
    if(n == 9)
      SWC_THROW_UNPOSSIBLE("reach here encoding vint64");
  }
  *(*bufp)++ = (uint8_t)(val & 0x7f);
} 
*/

SWC_CAN_INLINE 
uint64_t decode_vi64(const uint8_t** bufp, size_t* remainp) {
  uint64_t n = 0;
  for(uint8_t shift=0; ; shift+=7) {
    decode_needed_one(remainp);
    n |= (uint64_t)(**bufp & 0x7f) << shift;
    if(!(*(*bufp)++ & 0x80))
      return n;
    if(shift == 63)
      SWC_THROW_OVERRUN("vint64");
  };
}

SWC_CAN_INLINE 
uint64_t decode_vi64(const uint8_t** bufp) {
  size_t remain = 10;
  return decode_vi64(bufp, &remain);
}

SWC_CAN_INLINE 
size_t encoded_length_bytes32(int32_t len) {
  return len + 4;
}

SWC_CAN_INLINE 
void encode_bytes32(uint8_t** bufp, const void* data, int32_t len) {
  encode_i32(bufp, len);
  memcopy(bufp, (const uint8_t*)data, len);
}


SWC_CAN_INLINE 
uint8_t* decode_bytes32(const uint8_t** bufp, size_t* remainp, uint32_t* lenp) {
  *lenp = decode_i32(bufp, remainp);
  decode_needed(remainp, *lenp);
  uint8_t* out = (uint8_t *)*bufp;
  *bufp += *lenp;
  return out;
}

SWC_CAN_INLINE 
void encode_bytes(uint8_t** bufp, const void* data, int32_t len) {
  memcopy(bufp, (const uint8_t*)data, len);
}

SWC_CAN_INLINE 
uint8_t* decode_bytes(const uint8_t** bufp, size_t* remainp, uint32_t len) {
  decode_needed(remainp, len);
  uint8_t* out = (uint8_t *)*bufp;
  *bufp += len;
  return out;
}

SWC_CAN_INLINE 
size_t encoded_length_str16(const char* str) {
  return 2 + (!str ? 0 : strlen(str)) + 1;
}

SWC_CAN_INLINE 
size_t encoded_length_str16(const std::string& str) {
  return 2 + str.length() + 1;
}

SWC_CAN_INLINE 
void encode_str16(uint8_t** bufp, const void* str, uint16_t len) {
  encode_i16(bufp, len);
  if(len) 
    memcopy(bufp, (const uint8_t*)str, len);
  *(*bufp)++ = 0;
}

SWC_CAN_INLINE 
void encode_str16(uint8_t** bufp, const char* str) {
  uint16_t len = !str ? 0 : strlen(str);
  encode_str16(bufp, str, len);
}

SWC_CAN_INLINE 
const char* decode_str16(const uint8_t** bufp, size_t* remainp) {
  uint16_t len;
  return decode_str16(bufp, remainp, &len);
}

SWC_CAN_INLINE 
char* decode_str16(const uint8_t** bufp, size_t* remainp, uint16_t *lenp) {
  *lenp = decode_i16(bufp, remainp);
  decode_needed(remainp, *lenp+1);
  char* str = (char *)*bufp;
  *bufp += *lenp;
  if(*(*bufp)++ != 0)
    SWC_THROW_OVERRUN("str16");
  return str;
}

SWC_CAN_INLINE 
size_t encoded_length_vstr(size_t len) {
  return encoded_length_vi64(len) + len + 1;
}

SWC_CAN_INLINE 
size_t encoded_length_vstr(const char* s) {
  return encoded_length_vstr(s ? strlen(s) : 0);
}

SWC_CAN_INLINE 
size_t encoded_length_vstr(const std::string& s) {
  return encoded_length_vstr(s.length());
}

SWC_CAN_INLINE 
void encode_vstr(uint8_t** bufp, const void* buf, size_t len) {
  encode_vi64(bufp, len);
  if(len) 
    memcopy(bufp, (const uint8_t*)buf, len);
  *(*bufp)++ = 0;
}

SWC_CAN_INLINE 
void encode_vstr(uint8_t** bufp, const char* s) {
  encode_vstr(bufp, s, s ? strlen(s) : 0);
}

SWC_CAN_INLINE 
char* decode_vstr(const uint8_t** bufp, size_t* remainp) {
  uint32_t len;
  return decode_vstr(bufp, remainp, &len);
}

SWC_CAN_INLINE 
char* decode_vstr(const uint8_t** bufp, size_t* remainp, uint32_t* lenp) {
  *lenp = decode_vi64(bufp, remainp);
  decode_needed(remainp, *lenp+1);
  char* buf = (char *)*bufp;
  *bufp += *lenp;
  if(*(*bufp)++ != 0)
    SWC_THROW_OVERRUN("vstr");
  return buf;
}

SWC_CAN_INLINE 
void encode_double(uint8_t** bufp, double val) {
  int64_t lod = (int64_t)val;
  int64_t rod = (int64_t)((val - (double)lod) * (double)1000000000000000000.00);
  encode_i64(bufp, lod);
  encode_i64(bufp, rod);
}

SWC_CAN_INLINE 
double decode_double(const uint8_t** bufp, size_t* remainp) {
  return (double)decode_i64(bufp, remainp) 
       + ((double)decode_i64(bufp, remainp) / (double)1000000000000000000.00);
}

SWC_CAN_INLINE 
int encoded_length_double() {
  return 16;
}

SWC_CAN_INLINE 
bool equal(double a, double b) {
  int64_t lod_a = (int64_t)a;
  int64_t rod_a = (int64_t)((a - (double)lod_a) * (double)1000000000000000000.00);
  double aprime = (double)lod_a + ((double)rod_a / (double)1000000000000000000.00);
  int64_t lod_b = (int64_t)b;
  int64_t rod_b = (int64_t)((b - (double)lod_b) * (double)1000000000000000000.00);
  double bprime = (double)lod_b + ((double)rod_b / (double)1000000000000000000.00);
  return aprime == bprime;
}



}}

# undef SWC_CAN_INLINE 
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/Serializable.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Compat.h"
#include "swcdb/core/Error.h"

namespace SWC {

SWC_SHOULD_INLINE
size_t Serializable::encoded_length() const {
  size_t length = encoded_length_internal();
  return Serialization::encoded_length_vi32(length) + length;
}

SWC_SHOULD_INLINE
void Serializable::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, encoded_length_internal());
  encode_internal(bufp);
}

SWC_SHOULD_INLINE
void Serializable::decode(const uint8_t** bufp, size_t* remainp) {
  size_t len = Serialization::decode_vi32(bufp, remainp);
  if(len > *remainp) 
    SWC_THROWF(Error::PROTOCOL_ERROR, 
      "Buffer-Trunclated remain=%lld len=%lld", *remainp, len);
  *remainp -= len;

  const uint8_t* end = *bufp + len;
  decode_internal(bufp, &len);

  if(len)
    SWC_THROWF(Error::PROTOCOL_ERROR, 
      "Bad Decode missing=%lld in buffer", len);
  if(*bufp > end)
    SWC_THROWF(Error::PROTOCOL_ERROR, 
      "Bad Decode buffer overrun by=%lld", *bufp - end);
}


}
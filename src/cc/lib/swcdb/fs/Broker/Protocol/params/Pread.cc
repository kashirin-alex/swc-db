/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


PreadReq::PreadReq() {}

PreadReq::PreadReq(int32_t fd, uint64_t offset, uint32_t amount)
                  : fd(fd), offset(offset), amount(amount) {}

size_t PreadReq::internal_encoded_length() const {
  return 16;
}

void PreadReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, fd);
  Serialization::encode_i64(bufp, offset);
  Serialization::encode_i32(bufp, amount);
}

void PreadReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  offset = Serialization::decode_i64(bufp, remainp);
  amount = Serialization::decode_i32(bufp, remainp);
}


}}}}

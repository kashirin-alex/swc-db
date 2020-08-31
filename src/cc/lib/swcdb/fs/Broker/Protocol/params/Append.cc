/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


AppendReq::AppendReq() : fd(-1), flags(0) {}

AppendReq::AppendReq(int32_t fd, uint8_t flags)
                    : fd(fd), flags(flags) { }

size_t AppendReq::internal_encoded_length() const {
    return 5;
}

void AppendReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, fd);
  Serialization::encode_i8(bufp, flags);
}

void AppendReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  flags = Serialization::decode_i8(bufp, remainp);
}




AppendRsp::AppendRsp() {}

AppendRsp::AppendRsp(uint64_t offset, uint32_t amount)
                    : offset(offset), amount(amount) {}

size_t AppendRsp::internal_encoded_length() const {
    return 12;
}

void AppendRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i64(bufp, offset);
  Serialization::encode_i32(bufp, amount);
}

void AppendRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  offset = Serialization::decode_i64(bufp, remainp);
  amount = Serialization::decode_i32(bufp, remainp);
}


}}}}

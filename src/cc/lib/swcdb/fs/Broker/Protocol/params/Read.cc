/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


ReadReq::ReadReq() {}

ReadReq::ReadReq(int32_t fd, uint32_t amount)
                : fd(fd), amount(amount) {}

size_t ReadReq::internal_encoded_length() const {
    return 8;
}

void ReadReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, fd);
  Serialization::encode_i32(bufp, amount);
}

void ReadReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  amount = Serialization::decode_i32(bufp, remainp);
}




ReadRsp::ReadRsp() {}

ReadRsp::ReadRsp(uint64_t offset) 
                : offset(offset) {}


size_t ReadRsp::internal_encoded_length() const {
    return 8;
}

void ReadRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i64(bufp, offset);
}

void ReadRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  offset = Serialization::decode_i64(bufp, remainp);
}


}}}}

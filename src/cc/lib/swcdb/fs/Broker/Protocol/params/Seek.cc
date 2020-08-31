/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Seek.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {

SeekReq::SeekReq() {}

SeekReq::SeekReq(int32_t fd, size_t offset)
                : fd(fd), offset(offset) {}


size_t SeekReq::internal_encoded_length() const {
  return 12;
}

void SeekReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, fd);
  Serialization::encode_i64(bufp, offset);
}

void SeekReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  offset = Serialization::decode_i64(bufp, remainp);
}



SeekRsp::SeekRsp() {}

SeekRsp::SeekRsp(size_t offset) : offset(offset) {}

size_t SeekRsp::internal_encoded_length() const {
  return 8;
}

void SeekRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i64(bufp, offset);
}

void SeekRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  offset = Serialization::decode_i64(bufp, remainp);
}


}}}}

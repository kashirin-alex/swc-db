/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Close.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


CloseReq::CloseReq(): fd(-1) { }

CloseReq::CloseReq(int32_t fd) : fd(fd) { }

size_t CloseReq::internal_encoded_length() const {
  return 4;
}

void CloseReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, fd);
}

void CloseReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = (int32_t)Serialization::decode_i32(bufp, remainp);
}


}}}}

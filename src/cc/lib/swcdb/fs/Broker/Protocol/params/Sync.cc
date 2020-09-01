/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Sync.h"



namespace SWC { namespace FS { namespace Protocol { namespace Params {

SyncReq::SyncReq(): fd(-1) { }

SyncReq::SyncReq(int32_t fd) : fd(fd) { }

size_t SyncReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd);
}

void SyncReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
}

void SyncReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
}


}}}}

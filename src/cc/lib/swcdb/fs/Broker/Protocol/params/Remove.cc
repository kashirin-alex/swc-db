/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Remove.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


RemoveReq::RemoveReq() {}

RemoveReq::RemoveReq(const std::string& fname) : fname(fname) {}

size_t RemoveReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(fname.size());
}

void RemoveReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void RemoveReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fname.clear();
  fname.append(Serialization::decode_bytes_string(bufp, remainp));
}

}}}}

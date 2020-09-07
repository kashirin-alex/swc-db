/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Rmdir.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


RmdirReq::RmdirReq() {}

RmdirReq::RmdirReq(const std::string& dname) : dname(dname) {}

size_t RmdirReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(dname.size());
}

void RmdirReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, dname.c_str(), dname.size());
}

void RmdirReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  dname.clear();
  dname.append(Serialization::decode_bytes_string(bufp, remainp));
}


}}}}
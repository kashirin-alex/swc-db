/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {

ExistsReq::ExistsReq() {}

ExistsReq::ExistsReq(const std::string& fname) : fname(fname) {}

size_t ExistsReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(fname.size());
}

void ExistsReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void ExistsReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fname.clear();
  fname.append(Serialization::decode_bytes_string(bufp, remainp));
}




ExistsRsp::ExistsRsp() {}

ExistsRsp::ExistsRsp(bool exists) : exists(exists) {}

size_t ExistsRsp::internal_encoded_length() const {
  return 1;
}

void ExistsRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bool(bufp, exists);
}

void ExistsRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  exists = Serialization::decode_bool(bufp, remainp);
}



}}}}

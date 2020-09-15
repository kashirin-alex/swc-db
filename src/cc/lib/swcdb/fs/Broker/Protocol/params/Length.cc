/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Length.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {

LengthReq::LengthReq() {}

LengthReq::LengthReq(const std::string& fname) : fname(fname) {}

size_t LengthReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(fname.size());
}

void LengthReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void LengthReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fname.clear();
  fname.append(Serialization::decode_bytes_string(bufp, remainp));
}


LengthRsp::LengthRsp() {}

LengthRsp::LengthRsp(size_t length) : length(length) {}

size_t LengthRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi64(length);
}

void LengthRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, length);
}

void LengthRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  length = Serialization::decode_vi64(bufp, remainp);
}


}}}}

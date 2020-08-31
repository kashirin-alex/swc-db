/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Mkdirs.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


MkdirsReq::MkdirsReq() {}

MkdirsReq::MkdirsReq(const std::string& dirname) : dirname(dirname) {}

size_t MkdirsReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(dirname.size());
}

void MkdirsReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, dirname.c_str(), dirname.size());
}

void MkdirsReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  dirname.clear();
  dirname.append(Serialization::decode_bytes_string(bufp, remainp));
}


}}}}

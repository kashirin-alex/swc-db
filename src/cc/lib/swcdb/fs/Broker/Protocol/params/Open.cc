/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


OpenReq::OpenReq() {}

OpenReq::OpenReq(const std::string& fname, uint32_t flags, int32_t bufsz)
                : fname(fname), flags(flags), bufsz(bufsz) {}

size_t OpenReq::internal_encoded_length() const {
  return 8 + Serialization::encoded_length_bytes(fname.size());
}

void OpenReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, flags);
  Serialization::encode_i32(bufp, bufsz);
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void OpenReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flags = Serialization::decode_i32(bufp, remainp);
  bufsz = (int32_t)Serialization::decode_i32(bufp, remainp);
  fname.clear();
  fname.append(Serialization::decode_bytes_string(bufp, remainp));
}



OpenRsp::OpenRsp() {}

OpenRsp::OpenRsp(int32_t fd) : fd(fd) {}

size_t OpenRsp::internal_encoded_length() const {
  return 4;
}

void OpenRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, fd);
}

void OpenRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = (int32_t)Serialization::decode_i32(bufp, remainp);
}


}}}}

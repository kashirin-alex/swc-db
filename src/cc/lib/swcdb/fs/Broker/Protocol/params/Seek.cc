/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Seek.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Params {

SeekReq::SeekReq(): fd(-1) {}

SeekReq::SeekReq(int32_t fd, size_t offset)
                : fd(fd), offset(offset) {}

size_t SeekReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd)
       + Serialization::encoded_length_vi64(offset);
}

void SeekReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
  Serialization::encode_vi64(bufp, offset);
}

void SeekReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
}



SeekRsp::SeekRsp() {}

SeekRsp::SeekRsp(size_t offset) : offset(offset) {}

size_t SeekRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi64(offset);
}

void SeekRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, offset);
}

void SeekRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  offset = Serialization::decode_vi64(bufp, remainp);
}


}}}}

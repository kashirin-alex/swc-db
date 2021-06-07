/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t OpenReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(flags)
       + Serialization::encoded_length_vi32(bufsz)
       + Serialization::encoded_length_bytes(fname.size());
}

void OpenReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, flags);
  Serialization::encode_vi32(bufp, bufsz);
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void OpenReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flags = Serialization::decode_vi32(bufp, remainp);
  bufsz = Serialization::decode_vi32(bufp, remainp);
  fname = Serialization::decode_bytes_string(bufp, remainp);
}



size_t OpenRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd);
}

void OpenRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
}

void OpenRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
}


}}}}}

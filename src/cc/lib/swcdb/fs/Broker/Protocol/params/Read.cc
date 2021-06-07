/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Read.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t ReadReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd)
       + Serialization::encoded_length_vi32(amount);
}

void ReadReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
  Serialization::encode_vi32(bufp, amount);
}

void ReadReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
  amount = Serialization::decode_vi32(bufp, remainp);
}



size_t ReadRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi64(offset);
}

void ReadRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, offset);
}

void ReadRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  offset = Serialization::decode_vi64(bufp, remainp);
}


}}}}}

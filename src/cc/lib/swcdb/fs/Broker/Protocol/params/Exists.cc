/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t ExistsReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(fname.size());
}

void ExistsReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void ExistsReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fname = Serialization::decode_bytes_string(bufp, remainp);
}



void ExistsRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bool(bufp, exists);
}

void ExistsRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  exists = Serialization::decode_bool(bufp, remainp);
}



}}}}}

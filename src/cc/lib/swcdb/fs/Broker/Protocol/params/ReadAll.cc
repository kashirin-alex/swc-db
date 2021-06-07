/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t ReadAllReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(name.size());
}

void ReadAllReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, name.c_str(), name.size());
}

void ReadAllReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  name = Serialization::decode_bytes_string(bufp, remainp);
}


}}}}}

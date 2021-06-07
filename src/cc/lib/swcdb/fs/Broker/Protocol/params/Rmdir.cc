/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Rmdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t RmdirReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(dname.size());
}

void RmdirReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, dname.c_str(), dname.size());
}

void RmdirReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  dname = Serialization::decode_bytes_string(bufp, remainp);
}


}}}}}

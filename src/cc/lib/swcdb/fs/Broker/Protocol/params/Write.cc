/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Write.h"



namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t WriteReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(flags)
       + 1
       + Serialization::encoded_length_bytes(fname.size());
}

void WriteReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, flags);
  Serialization::encode_i8(bufp, replication);
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void WriteReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flags = Serialization::decode_vi32(bufp, remainp);
  replication = Serialization::decode_i8(bufp, remainp);
  fname = Serialization::decode_bytes_string(bufp, remainp);
}


}}}}}

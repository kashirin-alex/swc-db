/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t PreadReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd)
       + Serialization::encoded_length_vi64(offset)
       + Serialization::encoded_length_vi32(amount);
}

void PreadReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
  Serialization::encode_vi64(bufp, offset);
  Serialization::encode_vi32(bufp, amount);
}

void PreadReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
  amount = Serialization::decode_vi32(bufp, remainp);
}


}}}}}

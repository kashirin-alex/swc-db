/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Sync.h"



namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


size_t SyncReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd);
}

void SyncReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
}

void SyncReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
}


}}}}}

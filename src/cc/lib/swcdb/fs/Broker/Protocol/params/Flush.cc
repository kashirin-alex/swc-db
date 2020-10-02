/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Flush.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


FlushReq::FlushReq(): fd(-1) { }

FlushReq::FlushReq(int32_t fd): fd(fd) { }

size_t FlushReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(fd);
}

void FlushReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
}

void FlushReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
}


}}}}}

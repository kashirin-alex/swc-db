/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/SmartFd.h"
#include "swcdb/fs/Broker/Protocol/params/CombiPread.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


CombiPreadReq::CombiPreadReq() {}

CombiPreadReq::CombiPreadReq(const FS::SmartFd::Ptr& smartfd,
                             uint64_t offset, uint32_t amount)
                            : smartfd(smartfd), offset(offset), amount(amount) {}

size_t CombiPreadReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(smartfd->filepath().size())
       + Serialization::encoded_length_vi64(offset)
       + Serialization::encoded_length_vi32(amount);
}

void CombiPreadReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(
    bufp, smartfd->filepath().c_str(), smartfd->filepath().size());
  Serialization::encode_vi64(bufp, offset);
  Serialization::encode_vi32(bufp, amount);
}

void CombiPreadReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  smartfd = FS::SmartFd::make_ptr(
    Serialization::decode_bytes_string(bufp, remainp), 0);
  offset = Serialization::decode_vi64(bufp, remainp);
  amount = Serialization::decode_vi32(bufp, remainp);
}


}}}}}

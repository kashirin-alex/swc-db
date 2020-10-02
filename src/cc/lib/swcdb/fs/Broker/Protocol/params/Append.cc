/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


AppendReq::AppendReq(): fd(-1), flags(0) { }

AppendReq::AppendReq(int32_t fd, uint8_t flags)
                    : fd(fd), flags(flags) { }

size_t AppendReq::internal_encoded_length() const {
    return Serialization::encoded_length_vi32(fd) + 1;
}

void AppendReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, fd);
  Serialization::encode_i8(bufp, flags);
}

void AppendReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  fd = Serialization::decode_vi32(bufp, remainp);
  flags = Serialization::decode_i8(bufp, remainp);
}




AppendRsp::AppendRsp() {}

AppendRsp::AppendRsp(uint64_t offset, uint32_t amount)
                    : offset(offset), amount(amount) {}

size_t AppendRsp::internal_encoded_length() const {
    return Serialization::encoded_length_vi64(offset)
         + Serialization::encoded_length_vi32(amount);
}

void AppendRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, offset);
  Serialization::encode_vi32(bufp, amount);
}

void AppendRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  offset = Serialization::decode_vi64(bufp, remainp);
  amount = Serialization::decode_vi32(bufp, remainp);
}


}}}}}

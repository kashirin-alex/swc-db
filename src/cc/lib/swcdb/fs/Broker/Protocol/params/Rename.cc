/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Rename.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


RenameReq::RenameReq() {}

RenameReq::RenameReq(const std::string& from, const std::string& to)
                    : from(from), to(to) {}

size_t RenameReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(from.size())
       + Serialization::encoded_length_bytes(to.size());
}

void RenameReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, from.c_str(), from.size());
  Serialization::encode_bytes(bufp, to.c_str(), to.size());
}

void RenameReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  from = Serialization::decode_bytes_string(bufp, remainp);
  to = Serialization::decode_bytes_string(bufp, remainp);
}

}}}}}

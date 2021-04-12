/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Mkdirs.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


MkdirsReq::MkdirsReq() {}

MkdirsReq::MkdirsReq(const std::string& dirname) : dirname(dirname) {}

size_t MkdirsReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(dirname.size());
}

void MkdirsReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, dirname.c_str(), dirname.size());
}

void MkdirsReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  dirname = Serialization::decode_bytes_string(bufp, remainp);
}


}}}}}

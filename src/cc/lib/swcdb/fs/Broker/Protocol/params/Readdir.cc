/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


ReaddirReq::ReaddirReq() {}

ReaddirReq::ReaddirReq(const std::string& dirname) : dirname(dirname) {}

size_t ReaddirReq::internal_encoded_length() const {
  return Serialization::encoded_length_bytes(dirname.size());
}

void ReaddirReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, dirname.c_str(), dirname.size());
}

void ReaddirReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  dirname.clear();
  dirname.append(Serialization::decode_bytes_string(bufp, remainp));
}



size_t ReaddirRsp::internal_encoded_length() const {
  size_t length = Serialization::encoded_length_vi64(listing.size());
  for(auto& entry : listing)
    length += entry.encoded_length();
  return length;
}

void ReaddirRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, listing.size());
  for(auto& entry : listing)
    entry.encode(bufp);
}

void ReaddirRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  listing.clear();
  listing.resize(Serialization::decode_vi64(bufp, remainp));
  for(auto& entry : listing)
    entry.decode(bufp, remainp);
}


}}}}}

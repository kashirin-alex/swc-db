/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


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


ReaddirRsp::ReaddirRsp() {}

ReaddirRsp::ReaddirRsp(DirentList &listing) : m_listing(listing) {}

void ReaddirRsp::get_listing(DirentList &listing) {
  listing.clear();
  listing.assign(m_listing.begin(), m_listing.end());
}

size_t ReaddirRsp::internal_encoded_length() const {
  size_t length = 4;
  for (const Dirent &entry : m_listing)
    length += entry.encoded_length();
  return length;
}

void ReaddirRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, m_listing.size());
  for (const Dirent &entry : m_listing)
    entry.encode(bufp);
}

void ReaddirRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  int32_t count = Serialization::decode_i32(bufp, remainp);
  m_listing.clear();
  m_listing.resize(count);
  for (int32_t i=0; i<count; ++i)
    m_listing[i].decode(bufp, remainp);
}


}}}}

/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Write.h"



namespace SWC { namespace FS { namespace Protocol { namespace Params {


WriteReq::WriteReq() {}

WriteReq::WriteReq(const std::string& fname, uint32_t flags,
                   uint8_t replication, int64_t blksz)
                  : fname(fname), flags(flags),
                    replication(replication), blksz(blksz) {
}

size_t WriteReq::internal_encoded_length() const {
  return 13 + Serialization::encoded_length_bytes(fname.size());
}

void WriteReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, flags);
  Serialization::encode_i8(bufp, replication);
  Serialization::encode_i64(bufp, blksz);
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void WriteReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flags = Serialization::decode_i32(bufp, remainp);
  replication = Serialization::decode_i8(bufp, remainp);
  blksz = Serialization::decode_i64(bufp, remainp);
  fname.clear();
  fname.append(Serialization::decode_bytes_string(bufp, remainp));
}


}}}}

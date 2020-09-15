/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/params/Create.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


CreateReq::CreateReq() { }

CreateReq::CreateReq(const std::string& fname, uint32_t flags, int32_t bufsz,
                     uint8_t replication, int64_t blksz)
                    : fname(fname), flags(flags), bufsz(bufsz),
                      replication(replication), blksz(blksz) {}

size_t CreateReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(flags) 
       + Serialization::encoded_length_vi32(bufsz) 
       + 1
       + Serialization::encoded_length_vi64(blksz) 
       + Serialization::encoded_length_bytes(fname.size());
}

void CreateReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, flags);
  Serialization::encode_vi32(bufp, bufsz);
  Serialization::encode_i8(bufp, replication);
  Serialization::encode_vi64(bufp, blksz);
  Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
}

void CreateReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flags = Serialization::decode_vi32(bufp, remainp);
  bufsz = Serialization::decode_vi32(bufp, remainp);
  replication = Serialization::decode_i8(bufp, remainp);
  blksz = Serialization::decode_vi64(bufp, remainp);
  fname.clear();
  fname.append(Serialization::decode_bytes_string(bufp, remainp));
}


}}}}

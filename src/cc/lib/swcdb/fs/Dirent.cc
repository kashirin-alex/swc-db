/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/Dirent.h"

#include "swcdb/core/Serialization.h"

namespace SWC { namespace FS {

Dirent::~Dirent() { }

std::string Dirent::to_string() const {
  std::string s("Dirent(");
  s.append("name=");
  s.append(name);
  s.append(" length=");
  s.append(std::to_string(length));
  s.append(" is_dir=");
  s.append(std::to_string(is_dir));
  s.append(" modified=");
  s.append(std::to_string((int64_t)last_modification_time));
  
  s.append(")\n");
  return s;
}

size_t Dirent::internal_encoded_length() const {
  return 13 + Serialization::encoded_length_bytes(name.size());
}

void Dirent::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, name.c_str(), name.size());
  Serialization::encode_i64(bufp, length);
  Serialization::encode_i32(bufp, last_modification_time);
  Serialization::encode_bool(bufp, is_dir);
}

void Dirent::internal_decode(const uint8_t** bufp, size_t* remainp) {
  name = Serialization::decode_bytes_string(bufp, remainp);
  length = Serialization::decode_i64(bufp, remainp);
  last_modification_time = Serialization::decode_i32(bufp, remainp);
  is_dir = Serialization::decode_bool(bufp, remainp);
}

}}
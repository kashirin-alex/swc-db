/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/fs/Dirent.h"

#include "swcdb/core/Serialization.h"

namespace SWC{ namespace FS {

std::string Dirent::to_string(){
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

uint8_t Dirent::encoding_version() const {
  return 1;
}

size_t Dirent::encoded_length_internal() const {
  return 13 + Serialization::encoded_length_vstr(name);
}

void Dirent::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vstr(bufp, name);
  Serialization::encode_i64(bufp, length);
  Serialization::encode_i32(bufp, last_modification_time);
  Serialization::encode_bool(bufp, is_dir);
}

void Dirent::decode_internal(uint8_t version, const uint8_t **bufp, 
                             size_t *remainp) {
  name = Serialization::decode_vstr(bufp, remainp);
  length = Serialization::decode_i64(bufp, remainp);
  last_modification_time = Serialization::decode_i32(bufp, remainp);
  is_dir = Serialization::decode_bool(bufp, remainp);
}

}}
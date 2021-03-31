/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/Dirent.h"

#include "swcdb/core/Serialization.h"

namespace SWC { namespace FS {


Dirent::Dirent(const char* s, int64_t mod_time, bool is_dir, uint64_t length)
              : name(s), last_modification_time(mod_time),
                is_dir(is_dir), length(length) {
}

std::string Dirent::to_string() const {
  std::string s("Dirent(");
  s.append("name=");
  s.append(name);
  s.append(" modified=");
  s.append(std::to_string(last_modification_time));
  s.append(" is_dir=");
  s.append(std::to_string(is_dir));
  if(!is_dir) {
    s.append(" length=");
    s.append(std::to_string(length));
  }
  s.append(")\n");
  return s;
}

size_t Dirent::encoded_length() const noexcept {
  return Serialization::encoded_length_bytes(name.size())
       + Serialization::encoded_length_vi64(last_modification_time)
       + 1
       + (is_dir ? 0 : Serialization::encoded_length_vi64(length));
}

void Dirent::encode(uint8_t** bufp) const {
  Serialization::encode_bytes(bufp, name.c_str(), name.size());
  Serialization::encode_vi64(bufp, last_modification_time);
  Serialization::encode_bool(bufp, is_dir);
  if(!is_dir)
    Serialization::encode_vi64(bufp, length);
}

void Dirent::decode(const uint8_t** bufp, size_t* remainp) {
  name = Serialization::decode_bytes_string(bufp, remainp);
  last_modification_time = Serialization::decode_vi64(bufp, remainp);
  is_dir = Serialization::decode_bool(bufp, remainp);
  length = is_dir ? 0 : Serialization::decode_vi64(bufp, remainp);
}

}}

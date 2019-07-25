/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Dirent_h
#define swc_lib_fs_Dirent_h

#include <memory>
#include "swcdb/lib/core/String.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Serializable.h"

namespace SWC{ namespace FS {

/// Directory entry
class Dirent : public Serializable {

  public:
  /// File or directory name
  String name;
  /// Length of file
  uint64_t length {};
  /// Last modification time
  time_t last_modification_time {};
  /// Flag indicating if entry id a directory
  bool is_dir {};

  std::string to_string(){
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
  private:

  uint8_t encoding_version() const {
    return 1;
  }

  size_t encoded_length_internal() const {
    return 13 + Serialization::encoded_length_vstr(name);
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vstr(bufp, name);
    Serialization::encode_i64(bufp, length);
    Serialization::encode_i32(bufp, last_modification_time);
    Serialization::encode_bool(bufp, is_dir);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
    size_t *remainp) {
    name = Serialization::decode_vstr(bufp, remainp);
    length = Serialization::decode_i64(bufp, remainp);
    last_modification_time = Serialization::decode_i32(bufp, remainp);
    is_dir = Serialization::decode_bool(bufp, remainp);
  }
      
};

typedef std::vector<Dirent> DirentList;
}}


#endif  // swc_lib_fs_Dirent_h
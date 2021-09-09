/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Dirent_h
#define swcdb_fs_Dirent_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace FS {

/// Directory entry
struct Dirent final  {

  SWC_CAN_INLINE
  Dirent() noexcept { }

  SWC_CAN_INLINE
  Dirent(const char* s, int64_t mod_time, bool is_dir, uint64_t length)
        : name(s), last_modification_time(mod_time),
          is_dir(is_dir), length(length) {
  }

  SWC_CAN_INLINE
  Dirent(Dirent&& other) noexcept
        : name(std::move(other.name)),
          last_modification_time(other.last_modification_time),
          is_dir(other.is_dir), length(other.length) {
  }

  SWC_CAN_INLINE
  Dirent(const Dirent& other)
        : name(other.name),
          last_modification_time(other.last_modification_time),
          is_dir(other.is_dir), length(other.length) {
  }

  SWC_CAN_INLINE
  Dirent& operator=(Dirent&& other) noexcept {
    name = std::move(other.name);
    last_modification_time = other.last_modification_time;
    is_dir = other.is_dir;
    length = other.length;
    return *this;
  }

  /// File or Directory name
  std::string   name;
  /// Last modification time
  int64_t       last_modification_time;
  /// Whether a directory
  bool          is_dir;
  /// Length of file
  uint64_t      length;

  std::string to_string() const;

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    return Serialization::encoded_length_bytes(name.size())
         + Serialization::encoded_length_vi64(last_modification_time)
         + 1
         + (is_dir ? 0 : Serialization::encoded_length_vi64(length));
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_bytes(bufp, name.c_str(), name.size());
    Serialization::encode_vi64(bufp, last_modification_time);
    Serialization::encode_bool(bufp, is_dir);
    if(!is_dir)
      Serialization::encode_vi64(bufp, length);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    name = Serialization::decode_bytes_string(bufp, remainp);
    last_modification_time = Serialization::decode_vi64(bufp, remainp);
    is_dir = Serialization::decode_bool(bufp, remainp);
    length = is_dir ? 0 : Serialization::decode_vi64(bufp, remainp);
  }

};

typedef Core::Vector<Dirent> DirentList;
}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Dirent.cc"
#endif


#endif // swcdb_fs_Dirent_h

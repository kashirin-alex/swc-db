/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Dirent_h
#define swcdb_fs_Dirent_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace FS {

/// Directory entry
struct Dirent final  {

  SWC_CAN_INLINE
  Dirent() noexcept { }

  SWC_CAN_INLINE
  Dirent(const char* s, int64_t mod_time, bool is_dir, uint64_t length)
        : name(s), last_modification_time(mod_time),
          is_dir(is_dir), length(length) { }

  /// File or Directory name
  std::string   name;
  /// Last modification time
  int64_t       last_modification_time;
  /// Whether a directory
  bool          is_dir;
  /// Length of file
  uint64_t      length;

  std::string to_string() const;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

};

typedef std::vector<Dirent> DirentList;
}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Dirent.cc"
#endif


#endif // swcdb_fs_Dirent_h

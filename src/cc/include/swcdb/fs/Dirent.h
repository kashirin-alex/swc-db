/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Dirent_h
#define swcdb_fs_Dirent_h

#include <memory>
#include <vector>

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace FS {

/// Directory entry
class Dirent final : public Comm::Serializable {

  public:
  /// File or directory name
  std::string   name;
  /// Length of file
  uint64_t      length {};
  /// Last modification time
  time_t        last_modification_time {};
  /// Flag indicating if entry is a directory
  bool          is_dir {};

  std::string to_string() const;

  ~Dirent();
  
  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
      
};

typedef std::vector<Dirent> DirentList;
}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Dirent.cc"
#endif 


#endif // swcdb_fs_Dirent_h
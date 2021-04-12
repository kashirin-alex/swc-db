/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_SmartFd_h
#define swcdb_fs_SmartFd_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/MutexAtomic.h"

namespace SWC {


/**
 * \defgroup FileSystem The File-System Group
 * @brief A group with all related to SWC-DB File-System (libswcdb_fs).
 *
 *
 */



/**
 * @brief The SWC-DB File-System C++ namespace 'SWC::FS'
 *
 * \ingroup FileSystem
 */
namespace FS {


/// Smart FileDescriptor

struct SmartFd {
  public:

  typedef std::shared_ptr<SmartFd> Ptr;

  static Ptr make_ptr(const std::string& filepath, uint32_t flags,
                      int32_t fd=-1, uint64_t pos=0);

  static Ptr make_ptr(std::string&& filepath, uint32_t flags,
                      int32_t fd=-1, uint64_t pos=0);

  SmartFd(const std::string& filepath, uint32_t flags,
          int32_t fd=-1, uint64_t pos=0);

  SmartFd(std::string&& filepath, uint32_t flags,
          int32_t fd=-1, uint64_t pos=0) noexcept;

  virtual ~SmartFd() { }

  const std::string& filepath() const noexcept;

  void flags(uint32_t flags) noexcept;

  uint32_t flags() const noexcept;

  void fd(int32_t fd) noexcept;

  int32_t fd() const noexcept;

  bool valid() const noexcept;

  int32_t invalidate() noexcept;

  void pos(uint64_t pos) noexcept;

  uint64_t pos() const noexcept;

  void forward(uint64_t pos) noexcept;

  std::string to_string() const;

  void print(std::ostream& out) const;

  protected:

  const std::string   m_filepath;

  private:

  Core::Atomic<uint32_t> m_flags;
  Core::Atomic<int32_t>  m_fd;
  Core::Atomic<uint64_t> m_pos;

};

}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/SmartFd.cc"
#endif

#endif // swcdb_fs_SmartFd_h

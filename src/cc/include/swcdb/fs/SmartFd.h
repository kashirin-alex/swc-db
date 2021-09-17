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

class SmartFd {
  public:

  typedef std::shared_ptr<SmartFd> Ptr;

  SWC_CAN_INLINE
  static Ptr make_ptr(const std::string& filepath, uint32_t flags,
                      int32_t fd=-1, uint64_t pos=0) {
    return Ptr(new SmartFd(filepath, flags, fd, pos));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(std::string&& filepath, uint32_t flags,
                      int32_t fd=-1, uint64_t pos=0) {
    return Ptr(new SmartFd(std::move(filepath), flags, fd, pos));
  }

  SWC_CAN_INLINE
  SmartFd(const std::string& filepath, uint32_t flags,
          int32_t fd=-1, uint64_t pos=0)
          : m_filepath(filepath), m_flags(flags), m_fd(fd), m_pos(pos) {
  }

  SWC_CAN_INLINE
  SmartFd(std::string&& filepath, uint32_t flags,
          int32_t fd=-1, uint64_t pos=0) noexcept
          : m_filepath(std::move(filepath)),
            m_flags(flags), m_fd(fd), m_pos(pos) {
  }

  virtual ~SmartFd() noexcept;

  constexpr SWC_CAN_INLINE
  const std::string& filepath() const noexcept {
    return m_filepath;
  }

  constexpr SWC_CAN_INLINE
  void flags(uint32_t flags) noexcept {
    m_flags.store(flags);
  }

  constexpr SWC_CAN_INLINE
  uint32_t flags() const noexcept {
    return m_flags;
  }

  constexpr SWC_CAN_INLINE
  void fd(int32_t fd) noexcept {
    m_fd.store(fd);
  }

  constexpr SWC_CAN_INLINE
  int32_t fd() const noexcept {
    return m_fd;
  }

  constexpr SWC_CAN_INLINE
  bool valid() const noexcept {
    return m_fd != -1;
  }

  constexpr SWC_CAN_INLINE
  int32_t invalidate() noexcept {
    m_pos.store(0);
    return m_fd.exchange(-1);
  }

  constexpr SWC_CAN_INLINE
  void pos(uint64_t pos) noexcept {
    m_pos.store(pos);
  }

  constexpr SWC_CAN_INLINE
  uint64_t pos() const noexcept {
    return m_pos;
  }

  constexpr SWC_CAN_INLINE
  void forward(uint64_t nbytes) noexcept {
    m_pos.fetch_add(nbytes);
  }

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

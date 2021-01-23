/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/SmartFd.h"
#include "swcdb/core/String.h"


namespace SWC { namespace FS {


SWC_SHOULD_INLINE
SmartFd::Ptr SmartFd::make_ptr(const std::string& filepath, uint32_t flags,
                              int32_t fd, uint64_t pos){
  return std::make_shared<SmartFd>(filepath, flags, fd, pos);
}

SmartFd::SmartFd(const std::string& filepath, uint32_t flags,
                 int32_t fd, uint64_t pos)
                : m_filepath(filepath), m_flags(flags), m_fd(fd), m_pos(pos) {
}

SWC_SHOULD_INLINE
const std::string& SmartFd::filepath() const {
  return m_filepath;
}

void SmartFd::flags(uint32_t flags) {
  m_flags.store(flags);
}

uint32_t SmartFd::flags() const {
  return m_flags;
}

void SmartFd::fd(int32_t fd) {
  m_fd.store(fd);
}

int32_t SmartFd::fd() const {
  return m_fd;
}

bool SmartFd::valid() const {
  return m_fd != -1;
}

void SmartFd::pos(uint64_t pos) {
  m_pos.store(pos);
}

uint64_t SmartFd::pos() const {
  return m_pos;
}

void SmartFd::forward(uint64_t nbytes) {
  m_pos.fetch_add(nbytes);
}

std::string SmartFd::to_string() const {
  return format("SmartFd(filepath='%s' flags=%u fd=%d pos=%lu)",
    m_filepath.c_str(), m_flags.load(), m_fd.load(), m_pos.load());
}

void SmartFd::print(std::ostream& out) const {
  out << "SmartFd(filepath='" << m_filepath
      << "' flags=" << m_flags.load()
      << " fd=" << m_fd.load()
      << " pos=" << m_pos.load()
      << ')';
}


}}


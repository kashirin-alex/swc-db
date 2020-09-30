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

SmartFd::~SmartFd() { }

SWC_SHOULD_INLINE
const std::string& SmartFd::filepath() const { 
  return m_filepath; 
}

void SmartFd::flags(uint32_t flags) { 
  LockAtomic::Unique::scope lock(m_mutex);
  m_flags = flags; 
}

uint32_t SmartFd::flags() const { 
  LockAtomic::Unique::scope lock(m_mutex);
  return m_flags; 
}

void SmartFd::fd(int32_t fd) { 
  LockAtomic::Unique::scope lock(m_mutex);
  m_fd = fd; 
}

int32_t SmartFd::fd() const {
  LockAtomic::Unique::scope lock(m_mutex);
  return m_fd; 
}

void SmartFd::pos(uint64_t pos) {
  LockAtomic::Unique::scope lock(m_mutex);
  m_pos = pos; 
}
  
uint64_t SmartFd::pos() const { 
  LockAtomic::Unique::scope lock(m_mutex);
  return m_pos; 
}

bool SmartFd::valid() const { 
  LockAtomic::Unique::scope lock(m_mutex);
  return m_fd != -1; 
}

std::string SmartFd::to_string() const {
  LockAtomic::Unique::scope lock(m_mutex);
  return format("SmartFd(filepath=%s, flags=%u, fd=%d, pos=%lu)", 
                m_filepath.c_str(), m_flags, m_fd, m_pos);
}

void SmartFd::print(std::ostream& out) const {
  LockAtomic::Unique::scope lock(m_mutex);
  out << "SmartFd(filepath=" << m_filepath
      << ", flags=" << m_flags
      << ", fd=" << m_fd
      << ", pos=" << m_pos
      << ')';
}


}}


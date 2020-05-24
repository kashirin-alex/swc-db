/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/fs/SmartFd.h"
#include "swcdb/core/String.h"


namespace SWC{ namespace FS {


SWC_SHOULD_INLINE
SmartFd::Ptr SmartFd::make_ptr(const std::string &filepath, uint32_t flags, 
                              int32_t fd, uint64_t pos){
  return std::make_shared<SmartFd>(filepath, flags, fd, pos);
}

SmartFd::SmartFd(const std::string &filepath, uint32_t flags, 
                 int32_t fd, uint64_t pos)
                : m_filepath(filepath), m_flags(flags), m_fd(fd), m_pos(pos) {
}

SmartFd::~SmartFd() { }

SWC_SHOULD_INLINE
const std::string& SmartFd::filepath() const { 
  return m_filepath; 
}

SWC_SHOULD_INLINE
void SmartFd::flags(uint32_t flags) { 
  m_flags = flags; 
}

SWC_SHOULD_INLINE
uint32_t SmartFd::flags() const { 
  return m_flags; 
}

SWC_SHOULD_INLINE
void SmartFd::fd(int32_t fd) { 
  m_fd = fd; 
}

SWC_SHOULD_INLINE
int32_t SmartFd::fd() const {
  return m_fd; 
}

SWC_SHOULD_INLINE
void SmartFd::pos(uint64_t pos) {
  m_pos = pos; 
}
  
SWC_SHOULD_INLINE
uint64_t SmartFd::pos() const { 
  return m_pos; 
}

bool SmartFd::valid() const { 
  return m_fd != -1; 
}

std::string SmartFd::to_string() const {
  return format("SmartFd(filepath=%s, flags=%u, fd=%d, pos=%lu)", 
                         m_filepath.c_str(), m_flags, m_fd, m_pos);
}

}}


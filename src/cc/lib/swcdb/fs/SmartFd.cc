/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/fs/SmartFd.h"
#include "swcdb/core/String.h"


namespace SWC{ namespace FS {


SmartFd::Ptr SmartFd::make_ptr(const std::string &filepath, uint32_t flags, 
                              int32_t fd, uint64_t pos){
  return std::make_shared<SmartFd>(filepath, flags, fd, pos);
}

SmartFd::SmartFd(const std::string &filepath, uint32_t flags, 
                 int32_t fd, uint64_t pos)
                : m_filepath(filepath), m_flags(flags), m_fd(fd), m_pos(pos) {
}

SmartFd::operator SmartFd::Ptr() { 
  return shared_from_this();
}

SmartFd::~SmartFd() { }

const std::string& SmartFd::filepath() const { 
  return m_filepath; 
}

void SmartFd::flags(uint32_t flags) { 
  m_flags = flags; 
}

const uint32_t SmartFd::flags() const { 
  return m_flags; 
}

void SmartFd::fd(int32_t fd) { 
  m_fd = fd; 
}

const int32_t SmartFd::fd() const {
  return m_fd; 
}

void SmartFd::pos(uint64_t pos) {
  m_pos = pos; 
}
  
const uint64_t SmartFd::pos() const { 
  return m_pos; 
}

const bool SmartFd::valid() const { 
  return m_fd != -1; 
}

const std::string SmartFd::to_string() const {
  return format("SmartFd(filepath=%s, flags=%u, fd=%d, pos=%lu)", 
                         m_filepath.c_str(), m_flags, m_fd, m_pos);
}

}}


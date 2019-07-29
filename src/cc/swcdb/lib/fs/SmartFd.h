/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_SmartFd_h
#define swc_lib_fs_SmartFd_h

#include <memory>
#include "swcdb/lib/core/String.h"

namespace SWC{ namespace FS {

/// Smart FileDescriptor 

struct SmartFd;
typedef std::shared_ptr<SmartFd> SmartFdPtr;


struct SmartFd : std::enable_shared_from_this<SmartFd>{
  public:
  
  static SmartFdPtr make_ptr(const String &filepath, uint32_t flags, 
                            int32_t fd=-1, uint64_t pos=0){
    return std::make_shared<SmartFd>(filepath, flags, fd, pos);
  }

  SmartFd(const String &filepath, uint32_t flags, int32_t fd=-1, uint64_t pos=0): 
          m_filepath(filepath), m_flags(flags), m_fd(fd), m_pos(pos) {
  }

  operator SmartFdPtr(){ return shared_from_this(); }

  virtual ~SmartFd(){}

  const String& filepath() { return m_filepath; }

  void flags(uint32_t flags){ m_flags = flags; }
  uint32_t flags(){ return m_flags; }

  void fd(int32_t fd){ m_fd = fd; }
  int32_t fd(){ return m_fd; }

  void pos(uint64_t pos){ m_pos = pos; }
  uint64_t pos(){ return m_pos; }

  bool valid() { return m_fd != -1; }

  const String to_string(){
    return format("SmartFd(filepath=%s, flags=%u, fd=%d, pos=%lu)", 
                           m_filepath.c_str(), m_flags, m_fd, m_pos);
  }

  private:
  const String  m_filepath;
  uint32_t      m_flags;
  int32_t       m_fd;
  uint64_t      m_pos;
};

}}


#endif  // swc_lib_fs_SmartFd_h
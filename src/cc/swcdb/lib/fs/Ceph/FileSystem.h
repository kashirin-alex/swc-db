/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Ceph_FileSystem_h
#define swc_lib_fs_Ceph_FileSystem_h

#include <iostream>
#include "swcdb/lib/core/fs/FileSystem.h"

namespace SWC{ namespace FS {


class FileSystemCeph: public FileSystem {
  public:

  FileSystemCeph(){}
  
  virtual ~FileSystemCeph(){}

  Types::Fs get_type() override;

  
  const std::string to_string() override {
    return "FileSystemCeph";
  }

  
};


}}



#endif  // swc_lib_fs_Ceph_FileSystem_h
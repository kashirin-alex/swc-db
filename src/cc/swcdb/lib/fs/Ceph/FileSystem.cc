/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"

extern "C" SWC::FS::FileSystem* make_new_fs(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemCeph());
};


namespace SWC{ namespace FS {

Types::Fs FileSystemCeph::get_type() {
  return Types::Fs::CEPH;
};


}}
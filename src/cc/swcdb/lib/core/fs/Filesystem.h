/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_fs_systems_FileSystem_h
#define swc_core_fs_systems_FileSystem_h

#include <memory>

namespace SWC{ namespace Types {
enum Fs {
  NONE,
  CUSTOM,
  LOCAL,
  HADOOP,
  CEPH,
  HTFSBROKER
};
}}


namespace SWC { namespace FS {


class FileSystem {
  public:

  FileSystem() {}
  
  virtual ~FileSystem() {}

  virtual Types::Fs get_type() {
    return Types::Fs::NONE;
  };
  
  virtual const std::string to_string(){
    return "FS::BASE(NONE)";
  }


  //private:
};

typedef std::shared_ptr<FileSystem> FileSystemPtr;




}}

extern "C"{
typedef SWC::FS::FileSystem* make_new_fs_t();
SWC::FS::FileSystem* make_new_fs();
}

#endif  // swc_core_fs_systems_FileSystem_h
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Ceph_FileSystem_h
#define swc_fs_Ceph_FileSystem_h

#include <iostream>
#include "swcdb/fs/FileSystem.h"

namespace SWC{ namespace FS {

bool apply_ceph();


class FileSystemCeph final : public FileSystem {
  public:

  FileSystemCeph() 
    : FileSystem(
        Env::Config::settings()->get_str("swc.fs.ceph.path.root"),
        apply_ceph()
      )
  { }

  virtual ~FileSystemCeph(){}
  
  Types::Fs get_type() override;

  std::string to_string() override {
    return format(
      "(type=CEPH, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }


  
  bool exists(int &err, const std::string &name) override {
    std::string abspath;
    get_abspath(name, abspath);
    SWC_LOGF(LOG_DEBUG, "exists file='%s'", abspath);

    errno = 0;
    bool state = false; // ceph-exists(m_filesystem, abspath.c_str()) == -1);
    err = errno;
    return state;
  }

  void mkdirs(int &err, const std::string &name) override {
    std::string abspath;
    get_abspath(name, abspath);
    SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", abspath);
    
    errno = 0;
    //ceph-mkdirs(m_filesystem, abspath.c_str());
    err = errno;
  }

};


}}



extern "C" {
SWC::FS::FileSystem* fs_make_new_ceph();
void fs_apply_cfg_ceph(SWC::Env::Config::Ptr env);
}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Ceph/FileSystem.cc"
#endif 

#endif  // swc_fs_Ceph_FileSystem_h
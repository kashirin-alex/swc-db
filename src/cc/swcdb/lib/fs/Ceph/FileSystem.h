/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Ceph_FileSystem_h
#define swc_lib_fs_Ceph_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"

namespace SWC{ namespace FS {

bool apply_ceph();


class FileSystemCeph: public FileSystem {
  public:

  FileSystemCeph(s) 
    : FileSystem(
        settings->get<String>("swc.fs.ceph.path.root"),
        apply_ceph()
      )
  { }

  virtual ~FileSystemCeph(){}
  
  Types::Fs get_type() override;

  const std::string to_string() override {
    return format(
      "(type=CEPH, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }


  
  bool exists(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("exists file='%s'", abspath);

    errno = 0;
    bool state = false; // ceph-exists(m_filesystem, abspath.c_str()) == -1);
    err = errno;
    return state;
  }

  void mkdirs(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("mkdirs path='%s'", abspath);
    
    errno = 0;
    //ceph-mkdirs(m_filesystem, abspath.c_str());
    err = errno;
  }

};


}}



#endif  // swc_lib_fs_Ceph_FileSystem_h
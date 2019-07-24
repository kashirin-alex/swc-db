/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Ceph_FileSystem_h
#define swc_lib_fs_Ceph_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"

namespace SWC{ namespace FS {

void apply_ceph(Config::SettingsPtr settings);


class FileSystemCeph: public FileSystem {
  public:

  FileSystemCeph(
    Config::SettingsPtr settings) 
    : FileSystem(settings),
      path_root(
        normalize_pathname(settings->get<String>("swc.fs.ceph.path.root")))
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

  const std::string path_root;
};


}}



#endif  // swc_lib_fs_Ceph_FileSystem_h
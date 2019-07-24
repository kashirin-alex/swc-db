/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Local_FileSystem_h
#define swc_lib_fs_Local_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"

namespace SWC{ namespace FS {

void apply_local(Config::SettingsPtr settings);


class FileSystemLocal: public FileSystem {
  public:

  FileSystemLocal(
    Config::SettingsPtr settings) 
    : FileSystem(settings),
      path_root(
        normalize_pathname(settings->get<String>("swc.fs.local.path.root")))
  { }

  virtual ~FileSystemLocal(){}







  Types::Fs get_type() override;

  const std::string to_string() override {
    return format(
      "(type=LOCAL, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }

  const std::string path_root;
};


}}



#endif  // swc_lib_fs_Local_FileSystem_h
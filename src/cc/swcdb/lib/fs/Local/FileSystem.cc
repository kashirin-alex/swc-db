/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


bool apply_local() {
  EnvConfig::settings()->file_desc().add_options()
    ("swc.fs.local.path.root", str(""), "Local FileSystem's base root path")
    ("swc.fs.local.OnFileChange.cfg", str(), "Dyn-config file")
  ;
  EnvConfig::settings()->parse_file(
    EnvConfig::settings()->get<String>("swc.fs.local.cfg", ""),
    EnvConfig::settings()->get<String>("swc.fs.local.OnFileChange.cfg", "")
  );
  return true;
}


Types::Fs FileSystemLocal::get_type() {
  return Types::Fs::LOCAL;
};


}}




extern "C" SWC::FS::FileSystem* fs_make_new(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemLocal());
};

extern "C" bool fs_apply_cfg(){
  return SWC::FS::apply_local();
};

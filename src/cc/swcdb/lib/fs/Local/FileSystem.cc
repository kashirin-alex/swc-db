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


}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_local(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemLocal());
};
void fs_apply_cfg_local(SWC::EnvConfigPtr env){
  SWC::EnvConfig::set(env);
};
}

/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


void apply_local(Config::SettingsPtr settings) {
  settings->file_desc().add_options()
    ("swc.fs.local.path.root", str(""), "Local FileSystem's base root path")
    ("swc.fs.local.OnFileChange.file", str(), "Dyn-config file")
  ;
  settings->parse_file(
    settings->get<String>("swc.fs.cfg.local", ""),
    settings->get<String>("swc.fs.local.OnFileChange.file", "")
  );
}


Types::Fs FileSystemLocal::get_type() {
  return Types::Fs::LOCAL;
};


}}




extern "C" SWC::FS::FileSystem* fs_make_new(SWC::Config::SettingsPtr settings){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemLocal(settings));
};

extern "C" void fs_apply_cfg(SWC::Config::SettingsPtr settings){
  SWC::FS::apply_local(settings);
};

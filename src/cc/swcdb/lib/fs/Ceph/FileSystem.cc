/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


void apply_ceph(Config::SettingsPtr settings) {
  settings->file_desc().add_options()
    ("swc.fs.ceph.path.root", str(""), "Ceph FileSystem's base root path")
    ("swc.fs.ceph.OnFileChange.file", str(), "Dyn-config file")
  ;
  settings->parse_file(
    settings->get<String>("swc.fs.cfg.ceph", ""),
    settings->get<String>("swc.fs.ceph.OnFileChange.file", "")
  );
}


Types::Fs FileSystemCeph::get_type() {
  return Types::Fs::CEPH;
};


}}




extern "C" SWC::FS::FileSystem* fs_make_new(SWC::Config::SettingsPtr settings){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemCeph(settings));
};

extern "C" void fs_apply_cfg(SWC::Config::SettingsPtr settings){
  SWC::FS::apply_ceph(settings);
};
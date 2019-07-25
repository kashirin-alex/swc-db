/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


void apply_hadoop(Config::SettingsPtr settings) {
  settings->file_desc().add_options()
    ("swc.fs.hadoop.path.root", str(""), "Hadoop FileSystem's base root path")
    ("swc.fs.hadoop.OnFileChange.file", str(), "Dyn-config file")

    ("swc.fs.hadoop.namenode", strs(), "Namenode Host + optional(:Port), muliple")
    ("swc.fs.hadoop.namenode.port", i32(), "Namenode Port")
    ("swc.fs.hadoop.user", str(), "Hadoop user")
  ;
  settings->parse_file(
    settings->get<String>("swc.fs.cfg.hadoop", ""),
    settings->get<String>("swc.fs.hadoop.OnFileChange.file", "")
  );
}




Types::Fs FileSystemHadoop::get_type() {
  return Types::Fs::CEPH;
};


}}




extern "C" SWC::FS::FileSystem* fs_make_new(SWC::Config::SettingsPtr settings){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemHadoop(settings));
};

extern "C" void fs_apply_cfg(SWC::Config::SettingsPtr settings){
  SWC::FS::apply_hadoop(settings);
};
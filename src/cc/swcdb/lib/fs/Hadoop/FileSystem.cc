/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


bool apply_hadoop() {
  EnvConfig::settings()->file_desc().add_options()
    ("swc.fs.hadoop.path.root", str(""), "Hadoop FileSystem's base root path")
    ("swc.fs.hadoop.OnFileChange.file", str(), "Dyn-config file")

    ("swc.fs.hadoop.namenode", strs(), "Namenode Host + optional(:Port), muliple")
    ("swc.fs.hadoop.namenode.port", i32(), "Namenode Port")
    ("swc.fs.hadoop.user", str(), "Hadoop user")
  ;
  EnvConfig::settings()->parse_file(
    EnvConfig::settings()->get<String>("swc.fs.cfg.hadoop", ""),
    EnvConfig::settings()->get<String>("swc.fs.hadoop.OnFileChange.file", "")
  );
  return true;
}




Types::Fs FileSystemHadoop::get_type() {
  return Types::Fs::CEPH;
};


}}




extern "C" SWC::FS::FileSystem* fs_make_new(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemHadoop());
};

extern "C" bool fs_apply_cfg(){
  return SWC::FS::apply_hadoop();
};
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


bool apply_broker() {
  EnvConfig::settings()->file_desc().add_options()
    ("swc.fs.broker.OnFileChange.cfg", str(), "Dyn-config file")
    ("swc.fs.broker.host", str(), "FsBroker host (default resolve by hostname)") 
    ("swc.fs.broker.port", i32(17000), "FsBroker port")
    ("swc.fs.broker.handlers", i32(48), "Handlers for broker tasks")
  ;
  EnvConfig::settings()->parse_file(
    EnvConfig::settings()->get<String>("swc.fs.broker.cfg", ""),
    EnvConfig::settings()->get<String>("swc.fs.broker.OnFileChange.cfg", "")
  );
  return true;
}

}}




extern "C" SWC::FS::FileSystem* fs_make_new(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemBroker());
};

extern "C" bool fs_apply_cfg(){
  return SWC::FS::apply_broker();
};

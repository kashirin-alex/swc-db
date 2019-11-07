/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "FileSystem.h"


namespace SWC{ namespace FS {


bool apply_broker() {
  Env::Config::settings()->file_desc().add_options()
    ("swc.fs.broker.OnFileChange.cfg", str(), "Dyn-config file")
    ("swc.fs.broker.host", str(), "FsBroker host (default by hostname)") 
    ("swc.fs.broker.port", i32(17000), "FsBroker port")
    ("swc.fs.broker.handlers", i32(48), "Handlers for broker tasks")
    ("swc.fs.broker.timeout", g_i32(30000), "Default request timeout in ms")
    ("swc.fs.broker.timeout.bytes.ratio", g_i32(1000), 
     "Timeout ratio to bytes, bytes/ratio=ms added to default timeout")
  ;
  Env::Config::settings()->parse_file(
    Env::Config::settings()->get<String>("swc.fs.broker.cfg", ""),
    Env::Config::settings()->get<String>("swc.fs.broker.OnFileChange.cfg", "")
  );
  return true;
}

}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_broker(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemBroker());
};
void fs_apply_cfg_broker(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
};
}

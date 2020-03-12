/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/fs/FileSystem.h"


namespace SWC{ namespace FS {


bool apply_ceph() {
  settings->file_desc.add_options()
    ("swc.fs.ceph.path.root", str(""), "Ceph FileSystem's base root path")
    ("swc.fs.ceph.cfg.dyn", strs(), "Dyn-config file")
  ;
  settings->parse_file(
    settings->get_str("swc.fs.ceph.cfg", ""),
    "swc.fs.ceph.cfg.dyn"
  );
  return;
}


Types::Fs FileSystemCeph::get_type() {
  return Types::Fs::CEPH;
};

}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_ceph(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemCeph());
};
void fs_apply_cfg_ceph(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
};
}

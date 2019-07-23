/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_fs_Settings_h
#define swc_core_fs_Settings_h

#include "swcdb/lib/core/config/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_fs_options(){
  file_desc().add_options()
    ("swc.fs", str(), "FileSystem: local|hadoop|ceph|htfsbroker|custom")
    ("swc.fs.lib", str(), "Shared library path based on fs/FileSystem.h")

    ("swc.fs.root", str("swcdb/"), "SWC root path")

    ("swc.fs.ht.port", i16(15863), "Port of htFsBroker")
    ("swc.fs.hdfs.ConfDir", str(), "Hadoop configuration directory "
     "(e.g. /etc/hadoop/conf or /usr/lib/hadoop/conf)")
  
  ;
}

}}

#endif // swc_core_fs_Settings_h
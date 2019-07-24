/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Settings_h
#define swc_lib_fs_Settings_h

#include "swcdb/lib/core/config/Settings.h"

namespace SWC{ namespace Config {


inline void Settings::init_fs_options(){
  file_desc().add_options()
    ("swc.fs.path.data", str("swcdb/"), 
     "SWC-DB data-path, within the FS(specific) base-path")

    ("swc.fs", str(), "main FileSystem: local|hadoop|ceph|htfsbroker|custom")

    ("swc.fs.cfg.local", str(), "Specific cfg-file for FS-local")
    ("swc.fs.lib.local", str(), "FS-local Lib-path based on fs/FileSystem.h")

    ("swc.fs.cfg.hadoop", str(), "Specific cfg-file for FS-hadoop")
    ("swc.fs.lib.hadoop", str(), "FS-hadoop Lib-path based on fs/FileSystem.h")

    ("swc.fs.cfg.ceph", str(), "Specific cfg-file for FS-ceph")
    ("swc.fs.lib.ceph", str(), "FS-ceph Lib-path based on fs/FileSystem.h")

    ("swc.fs.cfg.htfsbroker", str(), "Specific cfg-file for FS-htfsbroker")
    ("swc.fs.lib.htfsbroker", str(), "FS-htfsbroker Lib-path based on fs/FileSystem.h")

    ("swc.fs.cfg.custom", str(), "Specific cfg-file for FS-custom")
    ("swc.fs.lib.custom", str(), "FS-custom Lib-path based on fs/FileSystem.h")


    /* 
    ("swc.fs.ht.port", i16(15863), "Port of htFsBroker")
    ("swc.fs.hdfs.ConfDir", str(), "Hadoop configuration directory "
     "(e.g. /etc/hadoop/conf or /usr/lib/hadoop/conf)")
     */
  
  ;
}

}}

#endif // swc_lib_fs_Settings_h
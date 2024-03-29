/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/config/Settings.h"

namespace SWC { namespace Config {


SWC_SHOULD_NOT_INLINE
void init_fs_options(Settings* settings) {
  settings->file_desc.add_options()
    ("swc.fs.path.data", str("swcdb/"),
     "SWC-DB data-path, within the FS(specific) base-path")

    ("swc.fs", str(), "main FileSystem: local|hadoop|hadoop_jvm|ceph|broker|custom")

    ("swc.fs.local.cfg", str(), "Specific cfg-file for FS-local")
    ("swc.fs.lib.local", str(), "FS-local Lib-path based on fs/FileSystem.h")

    ("swc.fs.hadoop.cfg", str(), "Specific cfg-file for FS-hadoop")
    ("swc.fs.lib.hadoop", str(), "FS-hadoop Lib-path based on fs/FileSystem.h")

    ("swc.fs.hadoop_jvm.cfg", str(), "Specific cfg-file for FS-hadoop_jvm")
    ("swc.fs.lib.hadoop_jvm", str(), "FS-hadoop-JVM Lib-path based on fs/FileSystem.h")

    ("swc.fs.ceph.cfg", str(), "Specific cfg-file for FS-ceph")
    ("swc.fs.lib.ceph", str(), "FS-ceph Lib-path based on fs/FileSystem.h")

    ("swc.fs.custom.cfg", str(), "Specific cfg-file for FS-custom")
    ("swc.fs.lib.custom", str(), "FS-custom Lib-path based on fs/FileSystem.h")

    ("swc.fs.broker.cfg", str(), "Specific cfg-file for FS-broker")
    ("swc.fs.broker.underlying", str(),
      "as main FileSystem, without 'broker': local|hadoop|ceph|custom")
    ("swc.fs.lib.broker", str(), "FS-broker Lib-path based on fs/FileSystem.h")

  ;
}

}}

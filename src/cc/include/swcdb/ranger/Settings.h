/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_ranger_Settings_h
#define swc_ranger_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/fs/Settings.h"
#include "swcdb/db/client/Settings.h"

#include "swcdb/db/Types/Encoding.h"

namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  init_fs_options();
  init_client_options();

  file_desc.add_options()
    ("swc.rgr.cfg", str(), "Specific cfg-file for Ranger")
    ("swc.rgr.cfg.dyn", strs(), "Specific dyn. cfg-file for Ranger")

    ("swc.rgr.reactors", i32(8), "Number of Communication Reactors")
    ("swc.rgr.workers", i32(32), "Number of Workers a Reactor")
    ("swc.rgr.handlers", i32(8), "Number of App Handlers")
    ("swc.rgr.maintenance.handlers", i32(2), "Number of Maintenance Handlers")
    ("swc.rgr.ram.allowed.percent", g_i32(33), 
     "Memory RSS % allowed without freeing/releasing")
    ("swc.rgr.ram.reserved.percent", g_i32(33), 
     "Memory Total % reserved, threshold of low-memory enter state")
    ("swc.rgr.ram.release.rate", g_i32(100), 
     "Memory release-rate (malloc dependable)")

    ("swc.rgr.id.validation.interval", g_i32(120000), 
     "Validation of Ranger-ID against Mngr(root)")

    ("swc.rgr.compaction.check.interval", g_i32(300000), 
     "Interval in ms for Compaction ")
    ("swc.rgr.compaction.read.ahead", g_i8(5), 
     "Allowed read-ahead scans per Range compaction")
    ("swc.rgr.compaction.range.max", g_i8(2), 
     "Max Allowed Ranges at a time for compaction")

    ("swc.rgr.Range.CellStore.count.max", g_i8(10), 
     "Schema default cellstore-max in range before range-split")  
    ("swc.rgr.Range.CellStore.size.max", g_i32(1000000000), 
     "Schema default cellstore-size")  
    ("swc.rgr.Range.CellStore.replication", g_i8(3), 
     "Schema default cellstore-replication (fs-dependent)") 
     
    ("swc.rgr.Range.block.size", g_i32(64000000), 
     "Schema default block-size")
    ("swc.rgr.Range.block.cells", g_i32(100000), 
     "Schema default block-cells")
    ("swc.rgr.Range.block.encoding", 
      g_enum(
        (int)Types::Encoding::ZSTD,
        0,
        Types::from_string_encoding,
        Types::repr_encoding), 
     "Schema default block-encoding NONE/ZSTD/SNAPPY/ZLIB")  
     
    ("swc.rgr.Range.CommitLog.rollout.ratio", g_i8(3), 
     "Schema default CommitLog new fragment Rollout Block Ratio")

    ("swc.rgr.Range.compaction.percent", g_i8(33), 
     "Schema default compact-percent threshold")
  ;

}

void Settings::init_post_cmd_args(){ }

}}

#endif // swc_ranger_Settings_h
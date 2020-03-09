/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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
    ("swc.rgr.ram.percent", g_i32(33), 
     "Memory RSS allowed without freeing/releasing")
    ("swc.rgr.ram.release.rate", g_i32(100), 
     "Memory release-rate (malloc dependable)")

    ("swc.rgr.id.validation.interval", g_i32(120000), 
     "Validation of Ranger-ID against Mngr(root)")

    ("swc.rgr.compaction.check.interval", g_i32(300000), 
     "Interval in ms for Compaction ")

    ("swc.rgr.Range.CellStore.count.max", g_i8(10), 
     "Number of cellstores allowed in range before range-split")  
    ("swc.rgr.Range.CellStore.size.max", g_i32(1000000000), 
     "Default CellStore size")  
    ("swc.rgr.Range.CellStore.replication", g_i8(3), 
     "Default CellStore replication (fs-dependent)") 
     
    ("swc.rgr.Range.block.size", g_i32(64000000), 
     "Default Block Size in bytes")  
    ("swc.rgr.Range.block.cells", g_i32(100000), 
     "Default Block Cells count")  
    ("swc.rgr.Range.block.encoding", 
      g_enum(
        (int)Types::Encoding::SNAPPY,
        0,
        Types::from_string_encoding,
        Types::repr_encoding), 
     "Default Block encoding NONE/SNAPPY/ZLIB")  
     
    ("swc.rgr.Range.compaction.percent", g_i8(33), 
     "Compaction threshold in % applied over size of either by cellstore or block")
  ;

}

void Settings::init_post_cmd_args(){ }

}}

#endif // swc_ranger_Settings_h
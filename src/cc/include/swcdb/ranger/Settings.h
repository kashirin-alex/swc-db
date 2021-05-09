/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_Settings_h
#define swcdb_ranger_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/fs/Settings.h"
#include "swcdb/db/client/Settings.h"
#include "swcdb/db/Types/Encoder.h"


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
    ("swc.rgr.clients.handlers", i32(8), "Number of DB-Client Handlers")

    ("swc.rgr.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.rgr.maintenance.handlers", i32(2), "Number of Maintenance Handlers")

    ("swc.rgr.ram.allowed.percent", g_i32(33),
     "Memory RSS % allowed without freeing/releasing")
    ("swc.rgr.ram.reserved.percent", g_i32(33),
     "Memory Total % reserved, threshold of low-memory enter state")
    ("swc.rgr.ram.release.rate", g_i32(100),
     "Memory release-rate (malloc dependable)")

    ("swc.rgr.metrics.enabled", boo(true),
     "Enable or Disable Metrics Monitoring")

    ("swc.rgr.metrics.report.interval", g_i32(300),
     "Metrics Reporting Interval in Seconds")

    ("swc.rgr.id.validation.interval", g_i32(120000),
     "Validation of Ranger-ID against Mngr(root)")

    ("swc.rgr.Range.req.update.concurrency", g_i8(1),
     "Max Allowed Concurrency a Range for Update Requests")

    ("swc.rgr.compaction.check.interval", g_i32(300000),
     "Interval in ms for Compaction ")
    ("swc.rgr.compaction.read.ahead", g_i8(5),
     "Allowed read-ahead scans per Range compaction")
    ("swc.rgr.compaction.range.max", g_i8(2),
     "Max Allowed Ranges at a time for compaction")
    ("swc.rgr.compaction.commitlog.max", g_i8(3),
     "Max Allowed Commitlog including Total-Range compactions")

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
        int(SWC_DEFAULT_STORAGE_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "Schema default block-encoding NONE/ZSTD/SNAPPY/ZLIB")

    ("swc.rgr.Range.CommitLog.rollout.ratio", g_i8(3),
     "Schema default CommitLog new fragment Rollout Block Ratio")
    ("swc.rgr.Range.CommitLog.Compact.cointervaling", g_i8(3),
     "The minimal cointervaling Fragments for Compaction")
    ("swc.rgr.Range.CommitLog.Fragment.preload", g_i8(2),
     "The number of CommitLog Fragment to preload")

    ("swc.rgr.Range.compaction.percent", g_i8(33),
     "Schema default compact-percent threshold")
  ;

}

void Settings::init_post_cmd_args() { }

}}

#endif // swcdb_ranger_Settings_h

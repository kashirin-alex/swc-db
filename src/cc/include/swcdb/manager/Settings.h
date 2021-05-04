/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Settings_h
#define swcdb_manager_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/core/Encoder.h"
#include "swcdb/fs/Settings.h"
#include "swcdb/db/client/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  init_fs_options();
  init_client_options();

  file_desc.add_options()
    ("swc.mngr.cfg", str(), "Specific cfg-file for Manager")
    ("swc.mngr.cfg.dyn", strs(), "Specific dyn. cfg-file for Manager")

    ("swc.mngr.reactors", i32(8), "Number of Communication Reactors")
    ("swc.mngr.workers", i32(32), "Number of Workers a Reactor")
    ("swc.mngr.port", i16(15000), "Manager port")
    ("swc.mngr.handlers", i32(8), "Number of App Handlers")

    ("swc.mngr.clients.handlers", i32(8), "Number of DB-Client Handlers")

    ("swc.mngr.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.mngr.metrics.enabled", boo(true),
     "Enable or Disable Metrics Monitoring")
    ("swc.mngr.metrics.report.interval", g_i32(300),
     "Metrics Reporting Interval in Seconds")

    ("swc.mngr.role.connection.probes", g_i32(3),
     "Number of tries Mngr tries to connect to other manager")
    ("swc.mngr.role.connection.timeout", g_i32(1000),
     "Timeout in milliseconds for each connection probe")
    ("swc.mngr.role.connection.fallback.failures", g_i32(10),
     "Fallback only after this number of retries to connect")

    ("swc.mngr.role.request.timeout", g_i32(20000),
     "Timeout in milliseconds for Mngr-Hosts state request, "
     "total=(connection.probes x connection.timeout + request.timeout) x hosts")

    ("swc.mngr.role.check.interval", g_i32(10000),
     "Check Interval for Mngr-Hosts-Status changes")

    ("swc.mngr.role.check.delay.fallback", g_i32(30000),
     "Delay of a Check in milliseconds on Mngr-Host disconnected")
    ("swc.mngr.role.check.delay.updated", g_i32(200),
     "Delay in milliseconds on Mngr-Hosts-Status changes")

    ("swc.mngr.ranges.assign.Rgr.remove.failures", g_i32(255),
     "Number of failures(establishing conn.x probes) after which the Ranger is removed")

    ("swc.mngr.ranges.assign.delay.onRangerChange", g_i32(30000),
     "Delay of Ranges Assignment Check in milliseconds on Ranger(on/off)")
    ("swc.mngr.ranges.assign.delay.afterColumnsInit", g_i32(30000),
     "Delay of Ranges Assignment Check in milliseconds follow columns init")
    ("swc.mngr.ranges.assign.interval.check", g_i32(60000),
     "Ranges assignment interval in milliseconds between checks")

    ("swc.mngr.ranges.assign.due", g_i32(100),
     "Total allowed ranges due on Ranger assignment")

    ("swc.mngr.column.health.interval.check", g_i32(300000),
     "Column Health Check Interval in ms")
    ("swc.mngr.column.health.checks", g_i32(2),
     "Number of concurrent Column Health Checks")

    ("swc.mngr.schema.replication", g_i8(3),
     "Save schema under N-replications (fs-dependent)")

    ("swc.mngr.rangers.resource.interval.check", g_i32(120000),
     "Rangers Resources check interval in ms")
    ("swc.mngr.rangers.range.rebalance.max", g_i8(1),
     "The Max allowed Ranges for rebalance on a Rangers Resources update")

  ;

}

void Settings::init_post_cmd_args() { }

}}

#endif // swcdb_manager_Settings_h
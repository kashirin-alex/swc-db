/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_broker_Settings_h
#define swcdb_broker_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/fs/Settings.h"
#include "swcdb/db/client/Settings.h"
#include "swcdb/db/Types/Encoder.h"


namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  init_client_options();

  file_desc.add_options()
    ("swc.bkr.cfg", str(), "Specific cfg-file for Broker")
    ("swc.bkr.cfg.dyn", strs(), "Specific dyn. cfg-file for Broker")

    ("swc.bkr.reactors", i32(8), "Number of Communication Reactors")
    ("swc.bkr.workers", i32(32), "Number of Workers a Reactor")
    ("swc.bkr.handlers", i32(32), "Number of App Handlers")
    ("swc.bkr.clients.handlers", i32(32), "Number of DB-Client Handlers")

    ("swc.bkr.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.bkr.ram.allowed.percent", g_i32(33),
     "Memory RSS % allowed without freeing/releasing")
    ("swc.bkr.ram.reserved.percent", g_i32(33),
     "Memory Total % reserved, threshold of low-memory enter state")
    ("swc.bkr.ram.release.rate", g_i32(100),
     "Memory release-rate (malloc dependable)")

    ("swc.bkr.metrics.enabled", boo(true),
     "Enable or Disable Metrics Monitoring")

    ("swc.bkr.metrics.report.interval", g_i32(300),
     "Metrics Reporting Interval in Seconds")
  ;

}

void Settings::init_post_cmd_args() { }

}}

#endif // swcdb_broker_Settings_h

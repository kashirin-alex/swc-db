/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_app_fsbroker_Settings_h
#define swcdb_app_fsbroker_Settings_h

#include "swcdb/core/Encoder.h"
#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/fs/Settings.h"
#include "swcdb/db/client/Settings.h"

namespace SWC { namespace Config {



void init_app_options(Settings* settings) {
  init_comm_options(settings);
  init_fs_options(settings);
  init_client_options(settings);

  settings->file_desc.add_options()
    ("swc.fs.broker.host", str(),
      "FsBroker host (default resolve by hostname)")
    ("swc.fs.broker.port", i16(14000),
      "FsBroker port")

    ("swc.FsBroker.cfg", str(),
      "Specific cfg-file for FsBroker")
    ("swc.FsBroker.cfg.dyn", strs(),
      "Specific dyn. cfg-file for FsBroker")
    ("swc.cfg.dyn.period", g_i32(60000),
     "Dynamic cfg-file check interval in ms, zero without")

    ("swc.FsBroker.concurrency.relative", boo(true),
     "Determined ratio by HW-Concurrency")
    ("swc.FsBroker.reactors", i32(4),
     "Number of Communication Reactors or HW-Concurrency a Reactor")
    ("swc.FsBroker.workers", i32(16),
     "Number of Workers a Reactor")
    ("swc.FsBroker.handlers", i32(64),
     "Number or HW-Concurrency base of App Handlers")

    ("swc.FsBroker.metrics.enabled", boo(true),
     "Enable or Disable Metrics Monitoring")
    ("swc.FsBroker.metrics.report.broker", boo(true),
     "Report Metrics via Broker Client")
    ("swc.FsBroker.metrics.report.interval", g_i32(300),
     "Metrics Reporting Interval in Seconds")

    ("swc.FsBroker.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")
  ;
  settings->alias("host", "swc.fs.broker.host");
}

void init_post_cmd_args(Settings* settings) {
  settings->parse_file(
    settings->get_str("swc.fs.broker.cfg", ""), nullptr
  );
  settings->parse_file(
    settings->get_str("swc.FsBroker.cfg", ""), "swc.FsBroker.cfg.dyn"
  );
}


}}

#endif // swcdb_app_fsbroker_Settings_h

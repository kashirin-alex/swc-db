/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_app_thriftbroker_Settings_h
#define swcdb_app_thriftbroker_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/fs/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_app_options() {
  init_comm_options();
  init_fs_options();
  init_client_options();

  file_desc.add_options()
    ("swc.ThriftBroker.cfg", str(),
      "Specific cfg-file for ThriftBroker")
    ("swc.ThriftBroker.cfg.dyn", strs(),
      "Specific dyn. cfg-file for ThriftBroker")

    ("swc.ThriftBroker.port", i16(18000),
      "ThriftBroker port")
    ("swc.ThriftBroker.transport", str("framed"),
      "ThriftBroker timeout")

    ("swc.ThriftBroker.workers", i32(32),
      "Number of Comm-Workers")
    ("swc.ThriftBroker.connections.max", i64(INT64_MAX),
      "The Max client Connections allowed , total= addrs X this")

    ("swc.ThriftBroker.timeout", i32(900000),
      "ThriftBroker timeout")
    ("swc.ThriftBroker.clients.handlers", i32(8),
      "The number of SWC-DB clients handlers")

    ("swc.ThriftBroker.ram.allowed.percent", g_i32(33),
     "Memory RSS % allowed without freeing/releasing")
    ("swc.ThriftBroker.ram.reserved.percent", g_i32(33),
     "Memory Total % reserved, threshold of low-memory enter state")
    ("swc.ThriftBroker.ram.release.rate", g_i32(100),
     "Memory release-rate (malloc dependable)")

    ("swc.ThriftBroker.metrics.enabled", boo(true),
     "Enable or Disable Metrics Monitoring")
    ("swc.ThriftBroker.metrics.report.interval", g_i32(300),
     "Metrics Reporting Interval in Seconds")
  ;
}

void Settings::init_post_cmd_args(){
  parse_file(
    get_str("swc.ThriftBroker.cfg", ""),
    "swc.ThriftBroker.cfg.dyn"
  );
}


}}

#endif // swcdb_app_fsbroker_Settings_h

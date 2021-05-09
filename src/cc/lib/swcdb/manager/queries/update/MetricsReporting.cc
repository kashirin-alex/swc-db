/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/queries/update/MetricsReporting.h"


namespace SWC { namespace Manager { namespace Metric {


Reporting::Reporting(const Comm::IoContextPtr& io,
                     Config::Property::V_GINT32::Ptr cfg_intval)
        : Common::Query::Update::Metric::Reporting(io, cfg_intval) {
}

void Reporting::configure_mngr(const char*,
                               const Comm::EndPoints& endpoints) {
  char hostname[256];
  if(gethostname(hostname, sizeof(hostname)) == -1)
    SWC_THROW(errno, "gethostname");

  auto level = Common::Query::Update::Metric::Reporting::configure(
    "swcdb", "mngr", hostname, endpoints,
    Comm::Protocol::Mngr::Command::MAX_CMD
  );

  auto fs = Env::FsInterface::fs();
  if(fs->statistics.enabled)
    level->metrics.emplace_back(new Item_FS(fs));
  // ++ Manager Metrics
}


}}} // namespace SWC::Manager::Metric

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fsbroker/queries/update/MetricsReporting.h"


namespace SWC { namespace FsBroker { namespace Metric {


Reporting::Reporting(const Comm::IoContextPtr& io,
                     Config::Property::V_GINT32::Ptr cfg_intval)
          : Common::Query::Update::Metric::Reporting(io, cfg_intval),
            fds(new Item_CountVolume("fds")) {
}

void Reporting::configure_fsbroker(const char*, const Comm::EndPoints& endpoints) {
  char hostname[256];
  if(gethostname(hostname, sizeof(hostname)) == -1)
    SWC_THROW(errno, "gethostname");

  auto level = Common::Query::Update::Metric::Reporting::configure(
    "swcdb", "fsbroker", hostname, endpoints,
    Comm::Protocol::FsBroker::Command::MAX_CMD
  );

  auto fs = Env::FsInterface::fs();
  if(fs->statistics.enabled)
    level->metrics.emplace_back(new Item_FS(fs));
  level->metrics.emplace_back(fds);
}



}}} // namespace SWC::FsBroker::Metric

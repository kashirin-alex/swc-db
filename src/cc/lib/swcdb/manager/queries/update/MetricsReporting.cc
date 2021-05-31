/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/queries/update/MetricsReporting.h"


namespace SWC { namespace Manager { namespace Metric {


Reporting::Reporting(const Comm::IoContextPtr& io)
        : Common::Query::Update::Metric::Reporting(
            Env::Clients::get(),
            io,
            Env::Config::settings()->get<Config::Property::V_GINT32>(
              "swc.mngr.metrics.report.interval"),
            Env::Config::settings()->get_bool(
              "swc.mngr.metrics.report.broker")
              ? client::Clients::BROKER : client::Clients::DEFAULT
          ),
          net(nullptr) {
}

void Reporting::configure_mngr(const char*,
                               const Comm::EndPoints& endpoints) {
  char hostname[256];
  if(gethostname(hostname, sizeof(hostname)) == -1)
    SWC_THROW(errno, "gethostname");

  auto level = Common::Query::Update::Metric::Reporting::configure(
    "swcdb", "mngr", hostname, endpoints
  );

  level->metrics.emplace_back(
    net = new Item_Net<Comm::Protocol::Mngr::Commands>(
      endpoints, Env::Config::settings()->get_bool("swc.comm.ssl")));

  const auto& fs = Env::FsInterface::fs();
  if(fs->statistics.enabled)
    level->metrics.emplace_back(new Item_FS(fs));
  // ++ Manager Metrics
}


}}} // namespace SWC::Manager::Metric

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/broker/queries/update/MetricsReporting.h"


namespace SWC { namespace Broker { namespace Metric {


Reporting::Reporting(const Comm::IoContextPtr& io,
                     Config::Property::V_GINT32::Ptr cfg_intval)
        : Common::Query::Update::Metric::Reporting(
            Env::Clients::get(), io, cfg_intval),
          net(nullptr) {
}

void Reporting::configure_bkr(const char*, const Comm::EndPoints& endpoints) {
  char hostname[256];
  if(gethostname(hostname, sizeof(hostname)) == -1)
    SWC_THROW(errno, "gethostname");

  auto level = Common::Query::Update::Metric::Reporting::configure(
    "swcdb", "bkr", hostname, endpoints
  );

  level->metrics.emplace_back(
    net = new Item_Net<Comm::Protocol::Bkr::Commands>(
      endpoints, Env::Config::settings()->get_bool("swc.comm.ssl")));

  // ++ Broker Metrics
}



}}} // namespace SWC::Broker::Metric

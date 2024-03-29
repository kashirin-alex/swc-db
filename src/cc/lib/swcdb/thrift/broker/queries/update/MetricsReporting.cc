/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/thrift/broker/queries/update/MetricsReporting.h"


namespace SWC { namespace ThriftBroker { namespace Metric {


Reporting::Reporting(const Comm::IoContextPtr& a_io,
                     Config::Property::Value_int32_g::Ptr a_cfg_intval)
        : Common::Query::Update::Metric::Reporting(
            Env::Clients::get(), a_io, a_cfg_intval),
          net(nullptr) {
}

void Reporting::configure_thriftbroker(const char*,
                                       const Comm::EndPoints& endpoints) {
  char hostname[256];
  if(gethostname(hostname, sizeof(hostname)) == -1)
    SWC_THROW(errno, "gethostname");

  auto level = Common::Query::Update::Metric::Reporting::configure(
    "swcdb", "thriftbroker", hostname, endpoints
  );

  level->metrics.emplace_back(
    net = new Item_Net<Commands>(endpoints, false));

  // ++ ThriftBroker Metrics
}



}}} // namespace SWC::ThriftBroker::Metric

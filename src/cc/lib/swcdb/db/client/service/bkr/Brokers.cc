/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/bkr/Brokers.h"

namespace SWC { namespace client {


Brokers::Brokers(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextBroker::Ptr& bkr_ctx)
    : queues(std::make_shared<Comm::client::ConnQueues>(
        std::make_shared<Comm::client::Serialized>(
          "BROKER",
          ioctx,
          bkr_ctx ? bkr_ctx : std::make_shared<ContextBroker>(settings)
        ),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.Bkr.connection.timeout"),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.Bkr.connection.probes"),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.Bkr.connection.keepalive"),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.request.again.delay")
      )),
      cfg_hosts(
        settings.get<Config::Property::V_GSTRINGS>("swc.bkr.host")) {
}


Comm::EndPoints Brokers::get_endpoints() const {
  return Comm::EndPoints();
}


}} //namespace SWC::client


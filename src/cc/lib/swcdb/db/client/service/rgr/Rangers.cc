/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/rgr/Rangers.h"

namespace SWC { namespace client {


Rangers::Rangers(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextRanger::Ptr& rgr_ctx)
    : queues(std::make_shared<Comm::client::ConnQueues>(
        std::make_shared<Comm::client::Serialized>(
          "RANGER",
          ioctx,
          rgr_ctx ? rgr_ctx : std::make_shared<ContextRanger>(settings)
        ),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Rgr.connection.timeout"),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Rgr.connection.probes"),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Rgr.connection.keepalive"),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.request.again.delay")
      )),
      cache(
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Rgr.range.res.expiry")) {
}

}} //namespace SWC::client

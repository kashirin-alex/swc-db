/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/rgr/Rangers.h"

namespace SWC { namespace client {


Rangers::Rangers(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextRanger::Ptr& rgr_ctx)
    : queues(new Comm::client::ConnQueues(
        Comm::client::Serialized::Ptr(new Comm::client::Serialized(
          settings,
          "RANGER",
          ioctx,
          rgr_ctx
            ? rgr_ctx
            : ContextRanger::Ptr(new ContextRanger(settings))
        )),
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.Rgr.connection.timeout"),
        settings.get<Config::Property::Value_uint16_g>(
          "swc.client.Rgr.connection.probes"),
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.Rgr.connection.keepalive"),
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.request.again.delay")
      )),
      cache(
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.Rgr.range.res.expiry")) {
}

}} //namespace SWC::client

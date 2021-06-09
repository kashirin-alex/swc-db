/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/mngr/Managers.h"

namespace SWC { namespace client {


Managers::Managers(const Config::Settings& settings,
                   Comm::IoContextPtr ioctx,
                   const ContextManager::Ptr& mngr_ctx)
    : queues(new Comm::client::ConnQueues(
        Comm::client::Serialized::Ptr(new Comm::client::Serialized(
          settings,
          "MANAGER",
          ioctx,
          mngr_ctx
            ? mngr_ctx
            : ContextManager::Ptr(new ContextManager(settings))
        )),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Mngr.connection.timeout"),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Mngr.connection.probes"),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Mngr.connection.keepalive"),
        settings.get<Config::Property::V_GINT32>(
          "swc.client.request.again.delay")
      )),
      groups(Mngr::Groups::Ptr(new Mngr::Groups(settings))->init()) {
}

}} //namespace SWC::client


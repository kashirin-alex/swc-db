/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"

namespace SWC { namespace client {

Comm::IoContextPtr default_io() {
  if(!Env::IoCtx::ok())
    Env::IoCtx::init(8);
  return Env::IoCtx::io();
}

Clients::Clients(Comm::IoContextPtr ioctx,
                 const ContextManager::Ptr& mngr_ctx,
                 const ContextRanger::Ptr& rgr_ctx)
    : running(true),
      mngrs_groups(std::make_shared<Mngr::Groups>()->init()),
      mngr(std::make_shared<Comm::client::ConnQueues>(
        std::make_shared<Comm::client::Serialized>(
          "MANAGER",
          ioctx ? ioctx: ioctx = default_io(),
          mngr_ctx ? mngr_ctx : std::make_shared<ContextManager>()
        ),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Mngr.connection.timeout"),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Mngr.connection.probes"),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Mngr.connection.keepalive"),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.request.again.delay")
      )),

      rgr(std::make_shared<Comm::client::ConnQueues>(
        std::make_shared<Comm::client::Serialized>(
          "RANGER",
          ioctx,
          rgr_ctx ? rgr_ctx : std::make_shared<ContextRanger>()
        ),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Rgr.connection.timeout"),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Rgr.connection.probes"),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Rgr.connection.keepalive"),
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.request.again.delay")
      )),

      schemas(std::make_shared<Schemas>(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry"))),

      rangers(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.client.Rgr.range.res.expiry")) {
}

void Clients::stop() {
  running.store(false);
  rgr->stop();
  mngr->stop();
}

} // namespace client



namespace Env {

void Clients::init(const client::Clients::Ptr& clients) {
  m_env = std::make_shared<Clients>(clients);
}

client::Clients::Ptr& Clients::get() {
  SWC_ASSERT(m_env);
  return m_env->m_clients;
}

const Clients& Clients::ref() noexcept {
  return *m_env.get();
}

void Clients::reset() noexcept {
  m_env = nullptr;
}

Clients::Clients(const client::Clients::Ptr& clients)
          : cfg_send_buff_sz(
              Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
                "swc.client.send.buffer")),
            cfg_send_ahead(
              Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
                "swc.client.send.ahead")),
            cfg_send_timeout(
              Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
                "swc.client.send.timeout")),
            cfg_send_timeout_ratio(
              Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
                "swc.client.send.timeout.bytes.ratio")),

            cfg_recv_buff_sz(
              Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
                "swc.client.recv.buffer")),
            cfg_recv_ahead(
              Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
                "swc.client.recv.ahead")),
            cfg_recv_timeout(
              Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
                "swc.client.recv.timeout")),

            m_clients(clients) {
}

}

} //namespace SWC


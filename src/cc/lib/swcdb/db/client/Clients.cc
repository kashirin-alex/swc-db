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


Clients::Clients(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextManager::Ptr& mngr_ctx,
                 const ContextRanger::Ptr& rgr_ctx,
                 const ContextBroker::Ptr& bkr_ctx)
    : running(true), flags(Flag::BROKER | Flag::SCHEMA),
      cfg_send_buff_sz(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.buffer")),
      cfg_send_ahead(
        settings.get<SWC::Config::Property::V_GUINT8>(
          "swc.client.send.ahead")),
      cfg_send_timeout(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.timeout")),
      cfg_send_timeout_ratio(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.timeout.bytes.ratio")),

      cfg_recv_buff_sz(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.recv.buffer")),
      cfg_recv_ahead(
        settings.get<SWC::Config::Property::V_GUINT8>(
          "swc.client.recv.ahead")),
      cfg_recv_timeout(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.recv.timeout")),

      schemas(
        this,
        settings.get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry")),
      managers(settings, ioctx ? ioctx: ioctx = default_io(), mngr_ctx),
      rangers(settings, ioctx, rgr_ctx),
      brokers(settings, ioctx, bkr_ctx) {
}

Clients::Clients(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextManager::Ptr& mngr_ctx,
                 const ContextRanger::Ptr& rgr_ctx)
    : running(true), flags(Flag::DEFAULT),
      cfg_send_buff_sz(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.buffer")),
      cfg_send_ahead(
        settings.get<SWC::Config::Property::V_GUINT8>(
          "swc.client.send.ahead")),
      cfg_send_timeout(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.timeout")),
      cfg_send_timeout_ratio(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.timeout.bytes.ratio")),

      cfg_recv_buff_sz(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.recv.buffer")),
      cfg_recv_ahead(
        settings.get<SWC::Config::Property::V_GUINT8>(
          "swc.client.recv.ahead")),
      cfg_recv_timeout(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.recv.timeout")),

      schemas(
        this,
        settings.get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry")),
      managers(settings, ioctx ? ioctx: ioctx = default_io(), mngr_ctx),
      rangers(settings, ioctx, rgr_ctx) {
}

Clients::Clients(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextBroker::Ptr& bkr_ctx)
    : running(true), flags(Flag::BROKER),
      cfg_send_buff_sz(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.buffer")),
      cfg_send_ahead(
        settings.get<SWC::Config::Property::V_GUINT8>(
          "swc.client.send.ahead")),
      cfg_send_timeout(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.timeout")),
      cfg_send_timeout_ratio(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.send.timeout.bytes.ratio")),

      cfg_recv_buff_sz(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.recv.buffer")),
      cfg_recv_ahead(
        settings.get<SWC::Config::Property::V_GUINT8>(
          "swc.client.recv.ahead")),
      cfg_recv_timeout(
        settings.get<SWC::Config::Property::V_GINT32>(
          "swc.client.recv.timeout")),

      schemas(
        this,
        settings.get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry")),
      brokers(settings, ioctx ? ioctx: ioctx = default_io(), bkr_ctx) {
}

void Clients::stop() {
  running.store(false);

  if(brokers.queues)
    brokers.queues->stop();
  if(rangers.queues)
    rangers.queues->stop();
  if(managers.queues)
    managers.queues->stop();
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

Clients::Clients(const client::Clients::Ptr& clients) noexcept
                : m_clients(clients) { }

}

} //namespace SWC


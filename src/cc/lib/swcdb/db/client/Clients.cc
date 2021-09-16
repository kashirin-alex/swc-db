/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"

namespace SWC { namespace client {



Clients::Ptr Clients::make(
          const Config::Settings& settings,
          const Comm::IoContextPtr& io_ctx,
          const ContextManager::Ptr& mngr_ctx,
          const ContextRanger::Ptr& rgr_ctx,
          const ContextBroker::Ptr& bkr_ctx) {
  return Clients::Ptr(new Clients(
    settings, io_ctx, mngr_ctx, rgr_ctx, bkr_ctx));
}

Clients::Ptr Clients::make(
          const Config::Settings& settings,
          const Comm::IoContextPtr& io_ctx,
          const ContextManager::Ptr& mngr_ctx,
          const ContextRanger::Ptr& rgr_ctx) {
  return Clients::Ptr(new Clients(
    settings, io_ctx, mngr_ctx, rgr_ctx));
}

Clients::Ptr Clients::make(
          const Config::Settings& settings,
          const Comm::IoContextPtr& io_ctx,
          const ContextBroker::Ptr& bkr_ctx) {
  return Clients::Ptr(new Clients(
    settings, io_ctx, bkr_ctx));
}



Clients::Clients(const Config::Settings& settings,
                 const Comm::IoContextPtr& a_io_ctx,
                 const ContextManager::Ptr& mngr_ctx,
                 const ContextRanger::Ptr& rgr_ctx,
                 const ContextBroker::Ptr& bkr_ctx)
    : running(true), flags(Flag::DEFAULT | Flag::BROKER | Flag::SCHEMA),
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

      io_ctx(a_io_ctx),
      schemas(
        this,
        settings.get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry")),
      managers(settings, io_ctx, mngr_ctx),
      rangers(settings, io_ctx, rgr_ctx),
      brokers(settings, io_ctx, bkr_ctx) {
}

Clients::Clients(const Config::Settings& settings,
                 const Comm::IoContextPtr& a_io_ctx,
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

      io_ctx(a_io_ctx),
      schemas(
        this,
        settings.get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry")),
      managers(settings, io_ctx, mngr_ctx),
      rangers(settings, io_ctx, rgr_ctx) {
}

Clients::Clients(const Config::Settings& settings,
                 const Comm::IoContextPtr& a_io_ctx,
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

      io_ctx(a_io_ctx),
      schemas(
        this,
        settings.get<Config::Property::V_GINT32>(
          "swc.client.schema.expiry")),
      brokers(settings, io_ctx, bkr_ctx) {
}

void Clients::stop_services() {
  running.store(false);

  if(brokers.queues)
    brokers.queues->stop();
  if(rangers.queues)
    rangers.queues->stop();
  if(managers.queues)
    managers.queues->stop();
}

void Clients::stop() {
  stop_services();
  stop_io();
}



} // namespace client



namespace Env {

void Clients::init(const client::Clients::Ptr& clients) {
  m_env.reset(new Clients(clients));
}

}

} //namespace SWC


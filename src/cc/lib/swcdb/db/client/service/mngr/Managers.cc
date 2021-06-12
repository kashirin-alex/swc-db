/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/service/mngr/Managers.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"

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

bool Managers::put(const ClientsPtr& clients,
                   const cid_t& cid, Comm::EndPoints& endpoints,
                   const Comm::client::ConnQueue::ReqBase::Ptr& req) {
  if(endpoints.empty()) {
    groups->select(cid, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping() || !req->valid()) {
        req->handle_no_conn();
      } else {
        Comm::Protocol::Mngr::Req::MngrActive::make(
          clients, cid, req)->run();
      }
      return false;
    }
  }
  queues->get(endpoints)->put(req);
  return true;
}

bool Managers::put_role_schemas(
                   const ClientsPtr& clients,
                   Comm::EndPoints& endpoints,
                   const Comm::client::ConnQueue::ReqBase::Ptr& req) {
  if(endpoints.empty()) {
    groups->select(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping() || !req->valid()) {
        req->handle_no_conn();
      } else {
        Comm::Protocol::Mngr::Req::MngrActive::make(
          clients, DB::Types::MngrRole::SCHEMAS, req)->run();
      }
      return false;
    }
  }
  queues->get(endpoints)->put(req);
  return true;
}



}} //namespace SWC::client


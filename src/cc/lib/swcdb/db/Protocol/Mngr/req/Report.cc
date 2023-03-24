/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/req/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


Report::Report(const SWC::client::Clients::Ptr& a_clients,
               Params::Report::Function func, const uint32_t timeout)
              : client::ConnQueue::ReqBase(Buffers::make(1)),
                clients(a_clients), endpoints() {
  cbp->append_i8(func);
  cbp->header.set(REPORT, timeout);
}

Report::Report(const SWC::client::Clients::Ptr& a_clients,
               const EndPoints& a_endpoints,
               Params::Report::Function func,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(Buffers::make(1)),
                clients(a_clients), endpoints(a_endpoints) {
  cbp->append_i8(func);
  cbp->header.set(REPORT, timeout);
}

Report::Report(const SWC::client::Clients::Ptr& a_clients,
               const Serializable& params,
               Params::Report::Function func,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  Buffers::make(params, 1, REPORT, timeout)
                ),
                clients(a_clients), endpoints() {
  cbp->append_i8(func);
}

void Report::handle_no_conn() {
  clear_endpoints();
  run();
}

void Report::clear_endpoints() {
  clients->remove_mngr(endpoints);
  endpoints.clear();
}





ClusterStatus::ClusterStatus(const SWC::client::Clients::Ptr& a_clients,
                             const EndPoints& a_endpoints,
                             ClusterStatus::Cb_t&& a_cb,
                             const uint32_t timeout)
                            : Report(
                                a_clients,
                                a_endpoints,
                                Params::Report::Function::CLUSTER_STATUS,
                                timeout
                              ), cb(std::move(a_cb)) {
}

bool ClusterStatus::run() {
  clients->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ClusterStatus::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR);
}

void ClusterStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  int err = ev->error;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }

  cb(req(), err);
}





ColumnStatus::ColumnStatus(const SWC::client::Clients::Ptr& a_clients,
                           const Params::Report::ReqColumnStatus& params,
                           ColumnStatus::Cb_t&& a_cb,
                           const uint32_t timeout)
                          : Report(
                              a_clients,
                              params,
                              Params::Report::Function::COLUMN_STATUS,
                              timeout
                            ),
                            cb(std::move(a_cb)), cid(params.cid) {
}

bool ColumnStatus::run() {
  if(clients->stopping()) {
    cb(req(), Error::CLIENT_STOPPING, Params::Report::RspColumnStatus());
    return false;
  }
  if(endpoints.empty()) {
    clients->get_mngr(cid, endpoints);
    if(endpoints.empty()) {
      MngrActive::make(clients, cid, shared_from_this())->run();
      return false;
    }
  }
  clients->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ColumnStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspColumnStatus rsp_params;
  int err = ev->error;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);
      if(!err)
        rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}





RangersStatus::RangersStatus(const SWC::client::Clients::Ptr& a_clients,
                             cid_t a_cid, RangersStatus::Cb_t&& a_cb,
                             const uint32_t timeout)
                            : Report(
                                a_clients,
                                Params::Report::Function::RANGERS_STATUS,
                                timeout
                              ), cb(std::move(a_cb)), cid(a_cid) {
}

bool RangersStatus::run() {
  if(clients->stopping()) {
    cb(req(), Error::CLIENT_STOPPING, Params::Report::RspRangersStatus());
    return false;
  }
  if(endpoints.empty()) {
    bool no_cid = cid == DB::Schema::NO_CID;
    if(no_cid)
      clients->get_mngr(DB::Types::MngrRole::RANGERS, endpoints);
    else
      clients->get_mngr(cid, endpoints);

    if(endpoints.empty()) {
      if(no_cid)
        MngrActive::make(
          clients, DB::Types::MngrRole::RANGERS, shared_from_this())->run();
      else
        MngrActive::make(clients, cid, shared_from_this())->run();
      return false;
    }
  }
  clients->get_mngr_queue(endpoints)->put(req());
  return true;
}

void RangersStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspRangersStatus rsp_params;
  int err = ev->error;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);
      if(!err)
        rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}





ManagersStatus::ManagersStatus(const SWC::client::Clients::Ptr& a_clients,
                               const EndPoints& a_endpoints,
                               ManagersStatus::Cb_t&& a_cb,
                               const uint32_t timeout)
                              : Report(
                                  a_clients,
                                  a_endpoints,
                                  Params::Report::Function::MANAGERS_STATUS,
                                  timeout
                                ), cb(std::move(a_cb)) {
}

bool ManagersStatus::run() {
  if(endpoints.empty()) {
    clients->get_mngr(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      MngrActive::make(
        clients, DB::Types::MngrRole::SCHEMAS, shared_from_this())->run();
      return false;
    }
  }
  clients->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ManagersStatus::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR, Params::Report::RspManagersStatus());
}

void ManagersStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspManagersStatus rsp_params;
  int err = ev->error;
  if(!err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      err = Serialization::decode_i32(&ptr, &remain);
      if(!err)
        rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }
  }

  cb(req(), err, rsp_params);
}





}}}}}

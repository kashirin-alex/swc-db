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


Report::Report(Params::Report::Function func, const uint32_t timeout)
              : client::ConnQueue::ReqBase(false, Buffers::make(1)) {
  cbp->append_i8(func);
  cbp->header.set(REPORT, timeout);
}

Report::Report(const EndPoints& endpoints,
               Params::Report::Function func,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(false, Buffers::make(1)),
                endpoints(endpoints) {
  cbp->append_i8(func);
  cbp->header.set(REPORT, timeout);
}

Report::Report(const Serializable& params,
               Params::Report::Function func,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  false,
                  Buffers::make(params, 1, REPORT, timeout)
                ) {
  cbp->append_i8(func);
}

void Report::handle_no_conn() {
  clear_endpoints();
  run();
}

void Report::clear_endpoints() {
  Env::Clients::get()->mngrs_groups->remove(endpoints);
  endpoints.clear();
}



SWC_SHOULD_INLINE
void ClusterStatus::request(const EndPoints& endpoints,
                            ClusterStatus::Cb_t&& cb,
                            const uint32_t timeout) {
  std::make_shared<ClusterStatus>(endpoints, std::move(cb), timeout)->run();
}

SWC_SHOULD_INLINE
ClusterStatus::Ptr
ClusterStatus::make(const EndPoints& endpoints,
                    ClusterStatus::Cb_t&& cb,
                    const uint32_t timeout) {
  return std::make_shared<ClusterStatus>(endpoints, std::move(cb), timeout);
}

ClusterStatus::ClusterStatus(const EndPoints& endpoints,
                             ClusterStatus::Cb_t&& cb,
                             const uint32_t timeout)
                            : Report(
                                endpoints,
                                Params::Report::Function::CLUSTER_STATUS,
                                timeout
                              ), cb(std::move(cb)) {
}

bool ClusterStatus::run() {
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ClusterStatus::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR);
}

void ClusterStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

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




SWC_SHOULD_INLINE
void ColumnStatus::request(cid_t cid, ColumnStatus::Cb_t&& cb,
                           const uint32_t timeout) {
  request(Params::Report::ReqColumnStatus(cid), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnStatus::request(const Params::Report::ReqColumnStatus& params,
                           ColumnStatus::Cb_t&& cb,
                           const uint32_t timeout) {
  std::make_shared<ColumnStatus>(params, std::move(cb), timeout)->run();
}

SWC_SHOULD_INLINE
ColumnStatus::Ptr
ColumnStatus::make(const Params::Report::ReqColumnStatus& params,
                   ColumnStatus::Cb_t&& cb, const uint32_t timeout) {
  return std::make_shared<ColumnStatus>(params, std::move(cb), timeout);
}

ColumnStatus::ColumnStatus(const Params::Report::ReqColumnStatus& params,
                           ColumnStatus::Cb_t&& cb,
                           const uint32_t timeout)
                          : Report(
                              params,
                              Params::Report::Function::COLUMN_STATUS,
                              timeout
                            ),
                            cb(std::move(cb)), cid(params.cid) {
}

bool ColumnStatus::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(cid, endpoints);
    if(endpoints.empty()) {
      MngrActive::make(cid, shared_from_this())->run();
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ColumnStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

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



SWC_SHOULD_INLINE
void RangersStatus::request(cid_t cid, RangersStatus::Cb_t&& cb,
                            const uint32_t timeout) {
  std::make_shared<RangersStatus>(cid, std::move(cb), timeout)->run();
}

SWC_SHOULD_INLINE
RangersStatus::Ptr
RangersStatus::make(cid_t cid, RangersStatus::Cb_t&& cb,
                    const uint32_t timeout) {
  return std::make_shared<RangersStatus>(cid, std::move(cb), timeout);
}

RangersStatus::RangersStatus(cid_t cid, RangersStatus::Cb_t&& cb,
                             const uint32_t timeout)
                            : Report(
                                Params::Report::Function::RANGERS_STATUS,
                                timeout
                              ), cb(std::move(cb)), cid(cid) {
}

bool RangersStatus::run() {
  if(endpoints.empty()) {
    bool no_cid = cid == DB::Schema::NO_CID;
    if(no_cid)
      Env::Clients::get()->mngrs_groups->select(
        DB::Types::MngrRole::RANGERS, endpoints);
    else
      Env::Clients::get()->mngrs_groups->select(cid, endpoints);

    if(endpoints.empty()) {
      if(no_cid)
        MngrActive::make(
          DB::Types::MngrRole::RANGERS, shared_from_this())->run();
      else
        MngrActive::make(cid, shared_from_this())->run();
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void RangersStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

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



SWC_SHOULD_INLINE
void ManagersStatus::request(const EndPoints& endpoints,
                             ManagersStatus::Cb_t&& cb,
                             const uint32_t timeout) {
  std::make_shared<ManagersStatus>(endpoints, std::move(cb), timeout)->run();
}

SWC_SHOULD_INLINE
ManagersStatus::Ptr
ManagersStatus::make(const EndPoints& endpoints,
                     ManagersStatus::Cb_t&& cb,
                     const uint32_t timeout) {
  return std::make_shared<ManagersStatus>(endpoints, std::move(cb), timeout);
}

ManagersStatus::ManagersStatus(const EndPoints& endpoints,
                               ManagersStatus::Cb_t&& cb,
                               const uint32_t timeout)
                              : Report(
                                  endpoints,
                                  Params::Report::Function::MANAGERS_STATUS,
                                  timeout
                                ), cb(std::move(cb)) {
}

bool ManagersStatus::run() {
  if(endpoints.empty()) {
    Env::Clients::get()->mngrs_groups->select(
      DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      MngrActive::make(
        DB::Types::MngrRole::SCHEMAS, shared_from_this())->run();
      return false;
    }
  }
  Env::Clients::get()->mngr->get(endpoints)->put(req());
  return true;
}

void ManagersStatus::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR, Params::Report::RspManagersStatus());
}

void ManagersStatus::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

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

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {



Report::Report(const SWC::client::Clients::Ptr& clients,
               const EndPoints& endpoints,
               Params::Report::Function func,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(Buffers::make(1)),
                clients(clients), endpoints(endpoints) {
  cbp->append_i8(func);
  cbp->header.set(REPORT, timeout);
}

Report::Report(const SWC::client::Clients::Ptr& clients,
               const EndPoints& endpoints,
               Params::Report::Function func,
               const Serializable& params,
               const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  Buffers::make(params, 1, REPORT, timeout)
                ),
                clients(clients), endpoints(endpoints) {
  cbp->append_i8(func);
}

bool Report::run() {
  clients->get_rgr_queue(endpoints)->put(req());
  return true;
}





ReportRes::ReportRes(const SWC::client::Clients::Ptr& clients,
                     const EndPoints& endpoints,
                     ReportRes::Cb_t&& cb,
                     const uint32_t timeout)
                     : Report(
                        clients,
                        endpoints,
                        Params::Report::Function::RESOURCES,
                        timeout
                      ), cb(std::move(cb)) {
}

void ReportRes::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR, Params::Report::RspRes());
}

void ReportRes::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspRes rsp_params;
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





ReportCids::ReportCids(const SWC::client::Clients::Ptr& clients,
                       const EndPoints& endpoints,
                       ReportCids::Cb_t&& cb,
                       const uint32_t timeout)
                      : Report(
                          clients,
                          endpoints,
                          Params::Report::Function::CIDS,
                          timeout
                        ), cb(std::move(cb)) {
}

void ReportCids::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR, Params::Report::RspCids());
}

void ReportCids::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspCids rsp_params;
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





ReportColumnRids::ReportColumnRids(const SWC::client::Clients::Ptr& clients,
                                   const EndPoints& endpoints,
                                   cid_t cid,
                                   ReportColumnRids::Cb_t&& cb,
                                   const uint32_t timeout)
                                  : Report(
                                      clients,
                                      endpoints,
                                      Params::Report::Function::COLUMN_RIDS,
                                      Params::Report::ReqColumn(cid),
                                      timeout
                                    ), cb(std::move(cb)) {
}

void ReportColumnRids::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR, Params::Report::RspColumnRids());
}

void ReportColumnRids::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspColumnRids rsp_params;
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





ReportColumnsRanges::ReportColumnsRanges(
            const SWC::client::Clients::Ptr& clients,
            const EndPoints& endpoints,
            ReportColumnsRanges::Cb_t&& cb,
            const uint32_t timeout)
            : Report(
                clients,
                endpoints,
                Params::Report::Function::COLUMNS_RANGES,
                timeout
              ), cb(std::move(cb)) {
}

ReportColumnsRanges::ReportColumnsRanges(
          const SWC::client::Clients::Ptr& clients,
          const EndPoints& endpoints,
          cid_t cid,
          ReportColumnsRanges::Cb_t&& cb,
          const uint32_t timeout)
          : Report(
              clients,
              endpoints,
              Params::Report::Function::COLUMN_RANGES,
              Params::Report::ReqColumn(cid),
              timeout
            ), cb(std::move(cb)) {
}

void ReportColumnsRanges::handle_no_conn() {
  cb(req(), Error::COMM_CONNECT_ERROR, Params::Report::RspColumnsRanges());
}

void ReportColumnsRanges::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::Report::RspColumnsRanges rsp_params;
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

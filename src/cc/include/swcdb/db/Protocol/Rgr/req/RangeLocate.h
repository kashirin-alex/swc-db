
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rgr_req_RangeLocate_h
#define swc_lib_db_protocol_rgr_req_RangeLocate_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

  
class RangeLocate: public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(client::ConnQueue::ReqBase::Ptr, 
                             const Params::RangeLocateRsp&)> Cb_t;

  static inline void request(const Params::RangeLocateReq& params,
                             const EndPoints& endpoints, 
                             const Cb_t cb, const uint32_t timeout = 10000) {
    std::make_shared<RangeLocate>(params, endpoints, cb, timeout)->run();
  }

  RangeLocate(const Params::RangeLocateReq& params, const EndPoints& endpoints,
              const Cb_t cb, const uint32_t timeout) 
              : client::ConnQueue::ReqBase(false), 
                endpoints(endpoints), cb(cb) {
    cbp = CommBuf::make(params);
    cbp->header.set(RANGE_LOCATE, timeout);
  }

  virtual ~RangeLocate(){}

  void handle_no_conn() override {
    cb(req(), Params::RangeLocateRsp(Error::COMM_NOT_CONNECTED));
  }

  bool run(uint32_t timeout=0) override {
    Env::Clients::get()->rgr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {

    if(ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    Params::RangeLocateRsp rsp_params;
    if(ev->type == Event::Type::ERROR) {
      rsp_params.err = ev->error;
      cb(req(), rsp_params);
      return;
    }

    try{
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      rsp_params.err = e.code();
    }
    cb(req(), rsp_params);
  }

  const EndPoints  endpoints;
  const Cb_t       cb;

};


}}}}

#endif // swc_lib_db_protocol_req_RangeLocate_h

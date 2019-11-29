
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rgr_req_RangeQueryUpdate_h
#define swc_lib_db_protocol_rgr_req_RangeQueryUpdate_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "../params/RangeQueryUpdate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

  
class RangeQueryUpdate: public Common::Req::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(Common::Req::ConnQueue::ReqBase::Ptr, 
                             Params::RangeQueryUpdateRsp)> Cb_t;
  typedef std::function<void()> Cb_no_conn_t;

  static inline void 
  request(const Params::RangeQueryUpdateReq& params, DynamicBuffer::Ptr buffer,
          const EndPoints& endpoints, Cb_no_conn_t cb_no_conn, const Cb_t cb, 
          const uint32_t timeout = 10000){
    std::make_shared<RangeQueryUpdate>(
      params, buffer, endpoints, cb_no_conn, cb, timeout)
      ->run();
  }


  RangeQueryUpdate(const Params::RangeQueryUpdateReq& params,
                   DynamicBuffer::Ptr buffer, const EndPoints& endpoints,
                   Cb_no_conn_t cb_no_conn, const Cb_t cb, 
                   const uint32_t timeout) 
                  : Common::Req::ConnQueue::ReqBase(false), 
                    endpoints(endpoints), cb_no_conn(cb_no_conn), cb(cb) {
    // timeout by buffer->fill() bytes ratio
    StaticBuffer snd_buf(buffer->base, buffer->fill(), false);
    cbp = CommBuf::make(params, snd_buf);
    cbp->header.set(RANGE_QUERY_UPDATE, timeout);
  }

  virtual ~RangeQueryUpdate(){}

  void handle_no_conn() override {
    cb_no_conn();
  }

  bool run(uint32_t timeout=0) override {
    Env::Clients::get()->rgr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    
    //std::cout << "RangeQueryUpdateRsp " << ev->to_str() << "\n";
    if(ev->type == Event::Type::DISCONNECT){
      handle_no_conn();
      return;
    }

    Params::RangeQueryUpdateRsp rsp_params;
    if(ev->type == Event::Type::ERROR){
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

  private:

  EndPoints     endpoints;
  Cb_no_conn_t  cb_no_conn;
  const Cb_t    cb;
};


}}}}

#endif // swc_lib_db_protocol_req_RangeLocate_h

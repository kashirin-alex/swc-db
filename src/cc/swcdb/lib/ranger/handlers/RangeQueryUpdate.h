/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_ranger_handlers_RangeQueryUpdate_h
#define swc_app_ranger_handlers_RangeQueryUpdate_h

#include "swcdb/lib/db/Protocol/Rgr/params/RangeQueryUpdate.h"
#include "../callbacks/RangeQueryUpdate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Handler {

class RangeQueryUpdate : public AppHandler {
  public:

  RangeQueryUpdate(ConnHandlerPtr conn, Event::Ptr ev)
                  : AppHandler(conn, ev) { }

  void run() override {

    int err = Error::OK;
    Params::RangeQueryUpdateReq params;
    server::Rgr::Range::Ptr range;
    StaticBuffer::Ptr buffer;

    try {
      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;
      params.decode(&ptr, &remain);

      range =  Env::RgrColumns::get()->get_range(
        err, params.cid, params.rid, false);
      
      if(range == nullptr || !range->is_loaded()){
        if(err == Error::OK)
          err = Error::RS_NOT_LOADED_RANGE;
      }
      if(err == Error::OK && m_ev->data_ext.size == 0) {
        err = Error::INVALID_ARGUMENT;
      } else {
        buffer = std::make_shared<StaticBuffer>(m_ev->data_ext);
      }
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }

    try{
      auto cb = std::make_shared<server::Rgr::Callback::RangeQueryUpdate>(
        m_conn, m_ev);
      if(err) {
        cb->response(err);
        return;
      }
      
      range->add(new server::Rgr::Range::ReqAdd(buffer, cb));
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

};
  

}}}}

#endif // swc_app_ranger_handlers_RangeQueryUpdate_h
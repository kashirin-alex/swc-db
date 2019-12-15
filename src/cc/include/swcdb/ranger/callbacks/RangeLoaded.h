/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeLoaded_h
#define swc_lib_ranger_callbacks_RangeLoaded_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeLoaded : public ResponseCallback {
  public:

  RangeLoaded(ConnHandlerPtr conn, Event::Ptr ev, 
              const int64_t cid, const int64_t rid)
            : ResponseCallback(conn, ev), cid(cid), rid(rid) {
    Env::RgrData::in_process(1);
  }

  virtual ~RangeLoaded() { }

  void response(int &err) override {
    if(err == Error::OK && Env::RgrData::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    Range::Ptr range;
    if(err == Error::OK) {
      range =  Env::RgrColumns::get()->get_range(err, cid, rid);
      if(err != Error::OK || range == nullptr || !range->is_loaded())
        err = Error::RS_NOT_LOADED_RANGE;
    }
    if(err != Error::OK)
      goto send_error;
    

    try {
      Protocol::Rgr::Params::RangeLoaded params;
      range->get_interval(params.interval);
      auto cbp = CommBuf::make(params, 4);
      cbp->header.initialize_from_request_header(m_ev->header);
      cbp->append_i32(err);
      m_conn->send_response(cbp);
      Env::RgrData::in_process(-1);
      return;
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = Error::COMM_SEND_ERROR;
    }
    
    send_error:
      Env::RgrColumns::get()->unload_range(err, cid, rid, 
        [berr=err, ptr=shared_from_this()]
        (int err){
          ptr->send_error(berr, "");
          Env::RgrData::in_process(-1);
        }
      );
    
  }

  private:
  const int64_t cid;
  const int64_t rid;

};


}
}}}
#endif // swc_lib_ranger_callbacks_RangeLoaded_h

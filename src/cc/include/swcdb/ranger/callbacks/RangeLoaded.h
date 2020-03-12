/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_callbacks_RangeLoaded_h
#define swc_ranger_callbacks_RangeLoaded_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLoaded : public ResponseCallback {
  public:

  RangeLoaded(ConnHandlerPtr conn, Event::Ptr ev, 
              const int64_t cid, const int64_t rid)
            : ResponseCallback(conn, ev), cid(cid), rid(rid) {
    RangerEnv::in_process(1);
  }

  virtual ~RangeLoaded() { }

  void response(int &err) override {
    if(err == Error::OK && RangerEnv::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    RangePtr range;
    if(err == Error::OK) {
      range =  RangerEnv::columns()->get_range(err, cid, rid);
      if(err != Error::OK || range == nullptr || !range->is_loaded())
        err = Error::RS_NOT_LOADED_RANGE;
    }
    if(err != Error::OK)
      goto send_error;
    

    try {
      Protocol::Rgr::Params::RangeLoaded params;
      if(params.intval = range->type == Types::Range::MASTER)
        range->get_interval(params.interval);
        
      auto cbp = CommBuf::make(params, 4);
      cbp->header.initialize_from_request_header(m_ev->header);
      cbp->append_i32(err);
      m_conn->send_response(cbp);
      RangerEnv::in_process(-1);
      return;
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = Error::COMM_SEND_ERROR;
    }
    
    send_error:
      RangerEnv::columns()->unload_range(err, cid, rid, 
        [berr=err, ptr=shared_from_this()]
        (int err){
          ptr->send_error(berr, "");
          RangerEnv::in_process(-1);
        }
      );
    
  }

  private:
  const int64_t cid;
  const int64_t rid;

};


}
}}
#endif // swc_ranger_callbacks_RangeLoaded_h

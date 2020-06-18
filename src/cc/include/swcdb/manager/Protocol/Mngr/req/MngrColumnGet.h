
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#ifndef swc_manager_Protocol_mngr_req_MngrColumnGet_h
#define swc_manager_Protocol_mngr_req_MngrColumnGet_h

#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

class MngrColumnGet : public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(int, const Params::ColumnGetRsp&)> Cb_t;

  MngrColumnGet(const Params::ColumnGetReq& params, const Cb_t& cb) 
               : cb(cb) {
    cbp = CommBuf::make(params);
    cbp->header.set(COLUMN_GET, 60000);
  }
  
  virtual ~MngrColumnGet() { }
  
  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == COLUMN_GET) {
      
      Params::ColumnGetRsp rsp_params;
      int err = ev->error != Error::OK? ev->error: ev->response_code();

      if(err == Error::OK){
        try{
          const uint8_t *ptr = ev->data.base+4;
          size_t remain = ev->data.size-4;
          rsp_params.decode(&ptr, &remain);
        } catch (Exception &e) {
          SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
          err = e.code();
        }
      }
      cb(err, rsp_params);
      was_called = true;
      return;
    }
  }

  private:
  Cb_t   cb;
  
};

}}}}

#endif // swc_manager_Protocol_mngr_req_MngrColumnGet_h


/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrColumnGet_h
#define swc_lib_db_protocol_req_MngrColumnGet_h

#include "swcdb/lib/db/Protocol/Mngr/params/ColumnGet.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

class MngrColumnGet : public Common::Req::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(int, Params::ColumnGetRsp)> Cb_t;

  MngrColumnGet(const Params::ColumnGetReq& params, Cb_t cb) : cb(cb) {
    CommHeader header(COLUMN_GET, 60000);
    cbp = CommBuf::make(header, params);
  }
  
  virtual ~MngrColumnGet() { }
  
  void handle(ConnHandlerPtr conn, Event::Ptr &ev) {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == COLUMN_GET) {
      
      Params::ColumnGetRsp rsp_params;
      int err = ev->error != Error::OK? ev->error: response_code(ev);

      if(err == Error::OK){
        try{
          const uint8_t *ptr = ev->data.base+4;
          size_t remain = ev->data.size-4;
          rsp_params.decode(&ptr, &remain);
        } catch (Exception &e) {
          HT_ERROR_OUT << e << HT_END;
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

#endif // swc_lib_db_protocol_req_MngrColumnGet_h

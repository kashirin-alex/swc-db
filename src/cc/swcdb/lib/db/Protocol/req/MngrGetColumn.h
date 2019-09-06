
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrGetColumn_h
#define swc_lib_db_protocol_req_MngrGetColumn_h

#include "swcdb/lib/db/Protocol/params/GetColumn.h"

namespace SWC {
namespace Protocol {
namespace Req {

class MngrGetColumn : public ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(int, Protocol::Params::GetColumnRsp)> Cb_t;

  MngrGetColumn(Params::GetColumnReq params, Cb_t cb) : cb(cb) {
    CommHeader header(Command::CLIENT_REQ_GET_COLUMN, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~MngrGetColumn() { }
  
  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    if(was_called || !is_rsp(conn, ev))
      return;

    if(ev->header.command == Protocol::Command::CLIENT_REQ_GET_COLUMN) {
      
      Protocol::Params::GetColumnRsp rsp_params;
      int err = ev->error != Error::OK? ev->error: Protocol::response_code(ev);

      if(err == Error::OK){
        try{
          const uint8_t *ptr = ev->payload+4;
          size_t remain = ev->payload_len-4;
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

    conn->do_close();
  }

  private:
  Cb_t                  cb;
  
};

}}}

#endif // swc_lib_db_protocol_req_MngrGetColumn_h


/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_ColumnCompact_h
#define swc_lib_db_protocol_rgr_req_ColumnCompact_h

#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class ColumnCompact : public client::ConnQueue::ReqBase  {
  public:

  ColumnCompact(int64_t cid) 
                : client::ConnQueue::ReqBase(false) {
    cbp = CommBuf::make(Params::ColumnCompactReq(cid));
    cbp->header.set(COLUMN_COMPACT, 60000);
  }
  
  virtual ~ColumnCompact() { }
  
  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {

    if(was_called)
      return;

    if(ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command != COLUMN_COMPACT)
      return;
    
    Params::ColumnCompactRsp rsp_params;
    if(ev->type == Event::Type::ERROR) {
      rsp_params.err = ev->error;
    } else {
      try{
        const uint8_t *ptr = ev->data.base;
        size_t remain = ev->data.size;
        rsp_params.decode(&ptr, &remain);
      } catch (Exception &e) {
        SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
        rsp_params.err = e.code();
      }
    }      
    if(!rsp_params.err)
      was_called = true;
    else 
      request_again();
  }

  void handle_no_conn() override { 
  }

};

}}}}

#endif // swc_lib_db_protocol_rgr_req_ColumnCompact_h

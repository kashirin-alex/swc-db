
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_ActiveMngr_h
#define swc_lib_db_protocol_req_ActiveMngr_h

#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "swcdb/lib/db/Protocol/params/ActiveMngrReq.h"
#include "swcdb/lib/db/Protocol/params/ActiveMngrRsp.h"


#include "swcdb/lib/db/Protocol/rsp/ActiveMngrRspCb.h"

namespace SWC {
namespace Protocol {
namespace Req {

class ActiveMngr : public DispatchHandler {
  public:

  ActiveMngr(client::ClientsPtr clients, size_t begin, size_t end)
            : clients(clients), begin(begin), end(end){}
  virtual ~ActiveMngr(){ }
  bool run(uint32_t timeout_ms=60000) override {
    
    do_request:

    SWC::EndPoints endpoints = clients->mngrs_groups->get_endpoints(begin, end);
    
    client::ClientConPtr conn = clients->mngr_service->get_connection(endpoints);
  
    Params::ActiveMngrReq params(begin, end);
    CommHeader header(Command::CLIENT_REQ_ACTIVE_MNGR, timeout_ms);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    bool ok = conn->send_request(cbp, shared_from_this()) == Error::OK;
    if(!ok)
      goto do_request;

    clients->mngr_service->preserve(conn);
    return ok;
  }

  void set_cb(Rsp::ActiveMngrRspCbPtr cb_hdlr){
    cb = cb_hdlr;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());

    bool ok = false;
    if(ev->error == Error::OK 
       && ev->header.command == Command::CLIENT_REQ_ACTIVE_MNGR){

      try {
        const uint8_t *ptr = ev->payload;
        size_t remain = ev->payload_len;

        Params::ActiveMngrRsp params;
        const uint8_t *base = ptr;
        params.decode(&ptr, &remain);
        
        if(params.available){
          cb->run(params.endpoints);
          return;
        }
        ok = true;

      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
      }
    }

    if(!ok)
      conn->do_close();

    run_within(conn->m_io_ctx, 500);

  }

  client::ClientsPtr clients;
  size_t begin; 
  size_t end;
  
  Rsp::ActiveMngrRspCbPtr cb;

};
typedef std::shared_ptr<ActiveMngr> ActiveMngrPtr;

}}}

#endif // swc_lib_db_protocol_req_ActiveMngr_h

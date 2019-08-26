
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_ActiveMngrBase_h
#define swc_lib_db_protocol_req_ActiveMngrBase_h

#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "../params/ActiveMngr.h"


namespace SWC {
namespace Protocol {
namespace Req {


class ActiveMngrBase : public DispatchHandler {
  public:

  ActiveMngrBase(size_t begin, size_t end) 
                : begin(begin), end(end), timeout_ms(timeout_ms){}

  virtual ~ActiveMngrBase(){ }

  bool run(uint32_t timeout_ms=60000) override {
    SWC::EndPoints endpoints 
      = EnvClients::get()->mngrs_groups->get_endpoints(begin, end);
    
    EnvClients::get()->mngr_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()]
      (client::ClientConPtr conn){
        if(conn == nullptr || !conn->is_open()){
          ptr->run_within(EnvClients::get()->mngr_service->io(), 200);
          return;
        }
        ptr->run(conn);
      },
      std::chrono::milliseconds(timeout_ms), 
      1
    );
    return true;
  }

  void run(ConnHandlerPtr conn) override {
    
    Protocol::Params::ActiveMngrReq params(begin, end);
    CommHeader header(Protocol::Command::CLIENT_REQ_ACTIVE_MNGR, timeout_ms);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    if(conn->send_request(cbp, shared_from_this()) != Error::OK)
      run_within(conn->m_io_ctx, 200);
    else
      EnvClients::get()->mngr_service->preserve(
        std::dynamic_pointer_cast<client::ConnHandlerClient>(conn));  // if ( ?ttl)
  }

  virtual void handle(ConnHandlerPtr conn, EventPtr &ev) {
    
    // HT_DEBUGF("ActiveMngr handle: %s", ev->to_str().c_str());

    bool ok = false;
    if(ev->error == Error::OK 
       && ev->header.command == Protocol::Command::CLIENT_REQ_ACTIVE_MNGR){

      try {
        const uint8_t *ptr = ev->payload;
        size_t remain = ev->payload_len;

        Protocol::Params::ActiveMngrRsp params;
        const uint8_t *base = ptr;
        params.decode(&ptr, &remain);
        
        if(params.available && params.endpoints.size() > 0){
          run(params.endpoints);
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

  virtual void run(const EndPoints& endpoints) = 0;


  size_t    begin; 
  size_t    end;
  uint32_t  timeout_ms;
};

}}}

#endif // swc_lib_db_protocol_req_ActiveMngr_h

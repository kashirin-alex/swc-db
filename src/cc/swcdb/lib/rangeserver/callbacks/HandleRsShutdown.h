
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_rangeserver_callbacks_HandleRsShutdown_h
#define swc_lib_rangeserver_callbacks_HandleRsShutdown_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"
#include "swcdb/lib/db/Protocol/params/AssignRsID.h"
#include <functional>

namespace SWC {
namespace server {
namespace RS {

class HandleRsShutdown: public Protocol::Rsp::ActiveMngrRspCb {
  public:

  HandleRsShutdown(EndPoints endpoints, 
                  client::ClientsPtr clients, 
                  Protocol::Req::ActiveMngrPtr mngr_active,
                  uint64_t &rs_id, std::function<void()> cb)
                : rs_endpoints(endpoints), 
                  Protocol::Rsp::ActiveMngrRspCb(clients, mngr_active),
                  rs_id(rs_id), cb(cb) {
  }

  virtual ~HandleRsShutdown(){}

  client::ClientConPtr m_conn;
  void run(Protocol::Params::HostEndPoints* host) override {

    std::cout << "HandleRsShutdown-RUN \n";
    std::cout << host->to_string() << "\n";

    m_conn = clients->mngr_service->get_connection(host->endpoints);
  
    Protocol::Params::AssignRsID params(
      rs_id, Protocol::Params::AssignRsID::Flag::RS_SHUTTINGDOWN, rs_endpoints);

    CommHeader header(Protocol::Command::RS_REQ_MNG_RS_ID, 60000);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    m_conn->send_request(cbp, shared_from_this());    
  }
  
  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
    
    std::cout << ev->to_str() << "\n";

    conn->do_close();
    if(ev->error == Error::OK 
       && ev->header.command == Protocol::Command::RS_REQ_MNG_RS_ID){

      if(Protocol::response_code(ev) == Error::OK){
        std::cout << "HandleRsShutdown: RSP-OK \n";
        cb();
        return;
      }
    }

    mngr_active->run_within(conn->m_io_ctx, 1000);
  }

  EndPoints rs_endpoints;
  uint64_t &rs_id;
  std::function<void()> cb;
};

}}}

#endif // swc_lib_rangeserver_callbacks_HandleRsAssign_h

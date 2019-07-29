
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_rangeserver_callbacks_HandleRsAssign_h
#define swc_lib_rangeserver_callbacks_HandleRsAssign_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"
#include "swcdb/lib/db/Protocol/params/AssignRsID.h"

namespace SWC {
namespace server {
namespace RS {

class HandleRsAssign: public Protocol::Rsp::ActiveMngrRspCb {
  public:

  HandleRsAssign(client::ClientsPtr clients, 
                 Protocol::Req::ActiveMngrPtr mngr_active,
                 Files::RsDataPtr rs_data)
                : Protocol::Rsp::ActiveMngrRspCb(clients, mngr_active),
                  rs_data(rs_data) {

    cfg_check_interval = Config::settings->get_ptr<gInt32t>(
      "swc.rs.id.validation.interval");
  }

  virtual ~HandleRsAssign(){}

  client::ClientConPtr m_conn;
  void run(EndPoints endpoints) override {

    m_conn = clients->mngr_service->get_connection(endpoints);
  
    Protocol::Params::AssignRsID params(
      0, Protocol::Params::AssignRsID::Flag::RS_REQ, rs_data->endpoints);

    CommHeader header(Protocol::Command::RS_REQ_MNG_RS_ID, 60000);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    m_conn->send_request(cbp, shared_from_this());

    //clients->mngr_service->preserve(conn);
    
  }
  
  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());

    bool ok = false;

    if(ev->error == Error::OK 
       && ev->header.command == Protocol::Command::RS_REQ_MNG_RS_ID){

      if(Protocol::response_code(ev) == Error::OK){
        std::cout << "HandleRsAssign: RSP-OK, chk-interval=" << cfg_check_interval->get() << " \n";
        conn->do_close();
        mngr_active->run_within(conn->m_io_ctx, cfg_check_interval->get());
        return;
      }

      try {
        const uint8_t *ptr = ev->payload;
        size_t remain = ev->payload_len;

        Protocol::Params::AssignRsID rsp_params;
        const uint8_t *base = ptr;
        rsp_params.decode(&ptr, &remain);

        std::cout << "HandleRsAssign: rs_id=" << rsp_params.rs_id
                                  << " flag=" << rsp_params.flag << "\n";
        
        if(rsp_params.flag == Protocol::Params::AssignRsID::Flag::MNGR_REREQ){
          mngr_active->run_within(conn->m_io_ctx, 50);

        } 
        else if(rsp_params.flag == Protocol::Params::AssignRsID::Flag::MNGR_ASSIGNED
          || rsp_params.flag == Protocol::Params::AssignRsID::Flag::MNGR_REASSIGN){
          
          Protocol::Params::AssignRsID params;
          if(rs_data->rs_id == 0 || rs_data->rs_id == rsp_params.rs_id
            || (rs_data->rs_id != rsp_params.rs_id 
                && rsp_params.flag == Protocol::Params::AssignRsID::Flag::MNGR_REASSIGN)){
            rs_data->rs_id = rsp_params.rs_id;
          
            params = Protocol::Params::AssignRsID(
              rs_data->rs_id, Protocol::Params::AssignRsID::Flag::RS_ACK, 
              rs_data->endpoints);
            std::cout << "HandleRsAssign: RS_ACK, rs_data=" << rs_data->to_string() << "\n";
         
          } else {
            params = Protocol::Params::AssignRsID(
              rs_data->rs_id, Protocol::Params::AssignRsID::Flag::RS_DISAGREE, 
              rs_data->endpoints);
            std::cout << "HandleRsAssign: RS_DISAGREE, rs_data=" << rs_data->to_string() << "\n";
          }

          CommHeader header(Protocol::Command::RS_REQ_MNG_RS_ID, 60000);
          CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
          params.encode(cbp->get_data_ptr_address());

          conn->send_request(cbp, shared_from_this());
          return;
       
        } // else Flag can be only MNGR_NOT_ACTIVE
        
        ok = true;

      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
      }
    }

    if(!ok)
      conn->do_close();
    mngr_active->run_within(conn->m_io_ctx, 1000);
  }

  Files::RsDataPtr rs_data;
  gInt32tPtr  cfg_check_interval;
};

}}}

#endif // swc_lib_rangeserver_callbacks_HandleRsAssign_h


/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_MngRsId_h
#define swc_lib_db_protocol_req_MngRsId_h

#include "ActiveMngrBase.h"
#include "swcdb/lib/db/Protocol/params/MngRsId.h"

namespace SWC {
namespace Protocol {
namespace Req {


class MngRsId: public ActiveMngrBase {

  public:

  MngRsId() 
          : ActiveMngrBase(1, 1), m_conn(nullptr), m_shutting_down(false),
            cfg_check_interval(
              Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rs.id.validation.interval")
            ) { }

  virtual ~MngRsId(){}

  void assign() {
    ActiveMngrBase::run();
  }

  void shutting_down(std::function<void()> cb) {
    m_cb = cb;
    m_shutting_down = true;
    ActiveMngrBase::run();
  }

  void run(const EndPoints& endpoints) override {

    if(m_conn != nullptr && m_conn->is_open()){
      make_request(nullptr, false);
      return;
    }
    Env::Clients::get()->mngr_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()]
      (client::ClientConPtr conn){
        std::dynamic_pointer_cast<MngRsId>(ptr)->make_request(conn, true);
      },
      std::chrono::milliseconds(20000), 
      1
    );
  }

  void make_request(client::ClientConPtr conn, bool new_conn) {
    if(new_conn)
      m_conn = conn;

    if(m_conn == nullptr || !m_conn->is_open()){
      m_conn = nullptr;
      if(!new_conn)
        ActiveMngrBase::run();
      else
        run_within(Env::Clients::get()->mngr_service->io(), 200);
      return;
    }
    
    Files::RsDataPtr rs_data = Env::RsData::get();
    Protocol::Params::MngRsId params(
      m_shutting_down ? rs_data->rs_id.load() : 0, 
      m_shutting_down ? Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN 
                      : Protocol::Params::MngRsId::Flag::RS_REQ, 
      rs_data->endpoints
    );

    CommHeader header(Protocol::Command::REQ_MNGR_MNG_RS_ID, 60000);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    if(m_conn->send_request(cbp,shared_from_this()) != Error::OK)
      run_within(Env::Clients::get()->mngr_service->io(), 500);
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
    
    if(ev->header.command == Protocol::Command::CLIENT_REQ_ACTIVE_MNGR){
      ActiveMngrBase::handle(conn, ev);
      return;
    }

    bool ok = false;

    if(ev->error == Error::OK 
       && ev->header.command == Protocol::Command::REQ_MNGR_MNG_RS_ID){

      if(Protocol::response_code(ev) == Error::OK){
        std::cout << "MngRsId: RSP-OK, chk-interval=" << cfg_check_interval->get() << " \n";
        conn->do_close();
        run_within(conn->m_io_ctx, cfg_check_interval->get());
        return;
      }

      try {
        const uint8_t *ptr = ev->payload;
        size_t remain = ev->payload_len;

        Protocol::Params::MngRsId rsp_params;
        const uint8_t *base = ptr;
        rsp_params.decode(&ptr, &remain);

        std::cout << "MngRsId: rs_id=" << rsp_params.rs_id
                  << " flag=" << rsp_params.flag << "\n";
        
        if(rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_REREQ){
          run_within(conn->m_io_ctx, 50);

        } 
        else if(
          rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_ASSIGNED
          || 
          rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_REASSIGN){

          Files::RsDataPtr rs_data = Env::RsData::get();
          Protocol::Params::MngRsId params;

          if(rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_ASSIGNED
             && rsp_params.fs != Env::FsInterface::interface()->get_type()){
            HT_ERRORF("RS's %s not matching with Mngr's FS-type=%d,"
                      " RS(shutting-down)",
                      Env::FsInterface::interface()->to_string().c_str(), 
                      (int)rsp_params.fs);
            params = Protocol::Params::MngRsId(
              rs_data->rs_id, Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN, 
              rs_data->endpoints);
            std::cout << "MngRsId: RS_SHUTTINGDOWN, rs_data=" 
                          << rs_data->to_string() << "\n";
            std::raise(SIGTERM);

          } else if(rs_data->rs_id == 0 || rs_data->rs_id == rsp_params.rs_id
            || (rs_data->rs_id != rsp_params.rs_id 
                && rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_REASSIGN)){
            rs_data->rs_id = rsp_params.rs_id;
          
            params = Protocol::Params::MngRsId(
              rs_data->rs_id, Protocol::Params::MngRsId::Flag::RS_ACK, 
              rs_data->endpoints);
            std::cout << "MngRsId: RS_ACK, rs_data=" 
                      << rs_data->to_string() << "\n";
          } else {

            params = Protocol::Params::MngRsId(
              rs_data->rs_id, Protocol::Params::MngRsId::Flag::RS_DISAGREE, 
              rs_data->endpoints);
            std::cout << "MngRsId: RS_DISAGREE, rs_data="
                      << rs_data->to_string() << "\n";
          }

          CommHeader header(Protocol::Command::REQ_MNGR_MNG_RS_ID, 60000);
          CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
          params.encode(cbp->get_data_ptr_address());

          conn->send_request(cbp, shared_from_this());
          return;
       
        }
        else if(rsp_params.flag == Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN) {
          std::cout << "HandleRsShutdown: RSP-OK \n";
          if(m_cb != 0)
             m_cb();
          else
            HT_WARN("Shutdown flag without Callback!");
          return;

        } 
          // else Flag can be only MNGR_NOT_ACTIVE
        
        ok = true;

      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
      }
    }

    if(!ok)
      conn->do_close();
    run_within(conn->m_io_ctx, 1000);
  }

  client::ClientConPtr  m_conn;
  bool                  m_shutting_down;
  
  const gInt32tPtr cfg_check_interval;

  std::function<void()> m_cb = 0;
};
typedef std::shared_ptr<MngRsId> MngRsIdPtr;

}}}

#endif // swc_lib_db_protocol_req_MngRsId_h


/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_MngRsId_h
#define swc_lib_db_protocol_req_MngRsId_h

#include "ActiveMngrRoute.h"
#include "swcdb/lib/db/Protocol/params/MngRsId.h"

namespace SWC {
namespace Protocol {
namespace Req {


class MngRsId: public ConnQueue::ReqBase {

  public:
  typedef std::shared_ptr<MngRsId> Ptr;

  class Scheduler : public std::enable_shared_from_this<Scheduler> {

    public:
    typedef std::shared_ptr<Scheduler> Ptr;

    Scheduler() :
      m_timer(
        std::make_shared<asio::high_resolution_timer>(*Env::IoCtx::io()->ptr())
      ),
      cfg_check_interval(
        Env::Config::settings()->get_ptr<gInt32t>(
           "swc.rs.id.validation.interval")) {
    }

    virtual ~Scheduler(){}

    void set(uint32_t ms=0) {
      std::lock_guard<std::mutex> lock(m_mutex);

      m_timer->cancel();
      if(Env::IoCtx::stopping())
        return;

      m_timer->expires_from_now(
        std::chrono::milliseconds(ms == 0?cfg_check_interval->get():ms));

      m_timer->async_wait(
        [ptr=shared_from_this()](const asio::error_code ec) {
          if (ec != asio::error::operation_aborted){
            assign(ptr);
          }
        }
      );
    }

    void stop(){
      std::lock_guard<std::mutex> lock(m_mutex);
      m_timer->cancel();
    }

    private:
    std::mutex              m_mutex;
    const gInt32tPtr        cfg_check_interval;
    const TimerPtr          m_timer;
  };
  
  void static assign(Scheduler::Ptr validator) {
    std::make_shared<MngRsId>(validator)->assign();
  }

  void static shutting_down(Scheduler::Ptr validator, 
                            std::function<void()> cb) {
    std::make_shared<MngRsId>(validator)->shutting_down(cb);
  }
  

  MngRsId(Scheduler::Ptr validator, CommBufPtr cbp=nullptr) 
          : ConnQueue::ReqBase(false, cbp),
            validator(validator) {
  }
  
  virtual ~MngRsId(){}

  void assign() {
    Files::RsDataPtr rs_data = Env::RsData::get();
    create(
      Protocol::Params::MngRsId(
        0, 
        Protocol::Params::MngRsId::Flag::RS_REQ, 
        rs_data->endpoints
      )
    )->run();
  }

  void shutting_down(std::function<void()> cb) {
    HT_DEBUGF("RS_SHUTTINGDOWN(req) %s", 
              Env::RsData::get()->to_string().c_str());
    
    Files::RsDataPtr rs_data = Env::RsData::get();
    Ptr req = create(
      Protocol::Params::MngRsId(
        rs_data->rs_id.load(), 
        Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN, 
        rs_data->endpoints
      )
    );
    req->cb_shutdown = cb;
    req->run();
  }

  void handle_no_conn() override { 
    if(was_called)
      return;
    clear_endpoints();
    assign();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      Env::Clients::get()->mngrs_groups->select(1, endpoints);
      if(endpoints.empty()){
        std::make_shared<ActiveMngrRoute>(1, shared_from_this())->run();
        return false;
      }
    }
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
    if(was_called)
      return;
    was_called = true;

    if(ev->error != Error::OK 
       || ev->header.command != Protocol::Command::REQ_MNGR_MNG_RS_ID){
      validator->set(1000);
      return;
    }

    if(Protocol::response_code(ev) == Error::OK){
      validator->set();
      return;
    }      
    
    Protocol::Params::MngRsId rsp_params;
    try {
      const uint8_t *ptr = ev->payload;
      size_t remain = ev->payload_len;
      rsp_params.decode(&ptr, &remain);
 
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
        
    HT_DEBUGF(" handle: flag=%d, %s", rsp_params.flag, ev->to_str().c_str());

    if(rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_REREQ){
      assign();
      return;
    }
    
    if(rsp_params.flag == Protocol::Params::MngRsId::Flag::RS_SHUTTINGDOWN) {
      HT_DEBUGF("RS_SHUTTINGDOWN %s", 
                Env::RsData::get()->to_string().c_str());
      if(cb_shutdown != 0)
        cb_shutdown();
      else
        HT_WARN("Shutdown flag without Callback!");
      return;
    }

    if(rsp_params.flag != Protocol::Params::MngRsId::Flag::MNGR_ASSIGNED
        &&
       rsp_params.flag != Protocol::Params::MngRsId::Flag::MNGR_REASSIGN){
      clear_endpoints();
      // remain Flag can be only MNGR_NOT_ACTIVE || no other action 
      validator->set(1000);
      return;
    }

    Files::RsDataPtr rs_data = Env::RsData::get();

    if(rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_ASSIGNED
       && rsp_params.fs != Env::FsInterface::interface()->get_type()){

      HT_ERRORF("RS's %s not matching with Mngr's FS-type=%d,"
                      "RS_SHUTTINGDOWN %s",
        Env::FsInterface::interface()->to_string().c_str(), 
        (int)rsp_params.fs, 
        rs_data->to_string().c_str()
      );
        
      std::raise(SIGTERM);
      return;
    }
    
    Protocol::Params::MngRsId::Flag flag;
    if(rs_data->rs_id == 0 || rs_data->rs_id == rsp_params.rs_id
       || (rs_data->rs_id != rsp_params.rs_id 
           && rsp_params.flag == Protocol::Params::MngRsId::Flag::MNGR_REASSIGN)){

      rs_data->rs_id = rsp_params.rs_id;
      flag = Protocol::Params::MngRsId::Flag::RS_ACK;
      HT_DEBUGF("RS_ACK %s", rs_data->to_string().c_str());
    } else {

      flag = Protocol::Params::MngRsId::Flag::RS_DISAGREE;
      HT_DEBUGF("RS_DISAGREE %s", rs_data->to_string().c_str());
    }

    create(
      Protocol::Params::MngRsId(rs_data->rs_id, flag, rs_data->endpoints)
    )->run();
  }
   
  std::function<void()> cb_shutdown = 0;
  
  private:
  
  Ptr create(Protocol::Params::MngRsId params) {
    CommHeader header(Protocol::Command::REQ_MNGR_MNG_RS_ID, 60000);
    CommBufPtr new_cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(new_cbp->get_data_ptr_address());
    return std::make_shared<MngRsId>(validator, new_cbp);
  }

  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Scheduler::Ptr  validator;
  EndPoints             endpoints;

};

}}}

#endif // swc_lib_db_protocol_req_MngRsId_h

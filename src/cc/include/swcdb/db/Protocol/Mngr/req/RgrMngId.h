
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_mngr_req_RgrMngId_h
#define swc_lib_db_protocol_mngr_req_RgrMngId_h

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RgrMngId.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


class RgrMngId: public Common::Req::ConnQueue::ReqBase {

  public:
  typedef std::shared_ptr<RgrMngId> Ptr;

  class Scheduler : public std::enable_shared_from_this<Scheduler> {

    public:
    typedef std::shared_ptr<Scheduler> Ptr;

    Scheduler() :
      m_timer(asio::high_resolution_timer(*Env::IoCtx::io()->ptr())),
      cfg_check_interval(
        Env::Config::settings()->get_ptr<gInt32t>(
           "swc.rgr.id.validation.interval")) {
    }

    virtual ~Scheduler(){}

    void set(uint32_t ms) {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(!m_run)
        return;
        
      m_timer.cancel();

      m_timer.expires_from_now(
        std::chrono::milliseconds(ms == 0?cfg_check_interval->get():ms));

      m_timer.async_wait(
        [ptr=shared_from_this()](const asio::error_code ec) {
          if (ec != asio::error::operation_aborted){
            assign(ptr);
          }
        }
      );
    }

    void stop(){
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_run) 
        m_timer.cancel();
      m_run = false;
    }

    private:
    const gInt32tPtr             cfg_check_interval;
    
    std::mutex                   m_mutex;
    asio::high_resolution_timer  m_timer;
    bool                         m_run;
  };
  
  void static assign(Scheduler::Ptr validator) {
    std::make_shared<RgrMngId>(validator)->assign();
  }

  void static shutting_down(Scheduler::Ptr validator, 
                            std::function<void()> cb) {
    auto rs_data = Env::RgrData::get();
    SWC_LOGF(LOG_DEBUG, "RS_SHUTTINGDOWN(req) %s",  rs_data->to_string().c_str());

    Ptr req = std::make_shared<RgrMngId>(
      validator, 
      create(
        Params::RgrMngId(
          rs_data->id.load(), 
          Params::RgrMngId::Flag::RS_SHUTTINGDOWN, 
          rs_data->endpoints
        )
      )
    );
    req->cb_shutdown = cb;
    req->run();
  }
  
  static CommBuf::Ptr create(const Params::RgrMngId& params) {
    auto cbp = CommBuf::make(params);
    cbp->header.set(RGR_MNG_ID, 60000);
    return cbp;
  }

  RgrMngId(Scheduler::Ptr validator, CommBuf::Ptr cbp=nullptr) 
          : Common::Req::ConnQueue::ReqBase(false, cbp),
            validator(validator) {
  }
  
  virtual ~RgrMngId(){}

  void assign() {
    if(Env::RgrData::is_shuttingdown())
      return;

    cbp = create(
      Params::RgrMngId(
        0, 
        Params::RgrMngId::Flag::RS_REQ, 
        Env::RgrData::get()->endpoints
      )
    );
    run();
  }


  void handle_no_conn() override {
    clear_endpoints();
    validator->set(200);
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      Env::Clients::get()->mngrs_groups->select(1, endpoints);
      if(endpoints.empty()){
        std::make_shared<MngrActive>(1, shared_from_this())->run();
        return false;
      }
    }
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {

    if(ev->error != Error::OK || ev->header.command != RGR_MNG_ID) {
      validator->set(1000);
      return;
    }

    if(response_code(ev) == Error::OK){
      validator->set(0);
      return;
    }      
    
    Params::RgrMngId rsp_params;
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);
 
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
        
    if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_REREQ){
      assign();
      return;
    }
    
    if(rsp_params.flag == Params::RgrMngId::Flag::RS_SHUTTINGDOWN) {
      SWC_LOGF(LOG_DEBUG, "RS_SHUTTINGDOWN %s", 
                Env::RgrData::get()->to_string().c_str());
      if(cb_shutdown != 0)
        cb_shutdown();
      else
        SWC_LOG(LOG_WARN, "Shutdown flag without Callback!");
      return;
    }

    if(rsp_params.flag != Params::RgrMngId::Flag::MNGR_ASSIGNED
        &&
       rsp_params.flag != Params::RgrMngId::Flag::MNGR_REASSIGN){
      clear_endpoints();
      // remain Flag can be only MNGR_NOT_ACTIVE || no other action 
      validator->set(1000);
      return;
    }

    auto rs_data = Env::RgrData::get();

    if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_ASSIGNED
       && rsp_params.fs != Env::FsInterface::interface()->get_type()){

      SWC_LOGF(LOG_ERROR, "Ranger's %s not matching with Mngr's FS-type=%d,"
                      "RS_SHUTTINGDOWN %s",
        Env::FsInterface::interface()->to_string().c_str(), 
        (int)rsp_params.fs, 
        rs_data->to_string().c_str()
      );
        
      std::raise(SIGTERM);
      return;
    }
    
    Params::RgrMngId::Flag flag;
    if(rs_data->id == 0 || rs_data->id == rsp_params.id || 
      (rs_data->id != rsp_params.id 
       && rsp_params.flag == Params::RgrMngId::Flag::MNGR_REASSIGN)){

      rs_data->id = rsp_params.id;
      flag = Params::RgrMngId::Flag::RS_ACK;
      SWC_LOGF(LOG_DEBUG, "RS_ACK %s", rs_data->to_string().c_str());
    } else {

      flag = Params::RgrMngId::Flag::RS_DISAGREE;
      SWC_LOGF(LOG_DEBUG, "RS_DISAGREE %s", rs_data->to_string().c_str());
    }

    cbp = create(
      Params::RgrMngId(rs_data->id, flag, rs_data->endpoints)
    );
    run();
  }
   
  std::function<void()> cb_shutdown = 0;
  
  private:

  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Scheduler::Ptr  validator;
  EndPoints             endpoints;

};

}}}}

#endif // swc_lib_db_protocol_mngr_req_RgrMngId_h

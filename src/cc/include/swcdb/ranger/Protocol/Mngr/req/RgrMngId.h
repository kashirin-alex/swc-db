
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_ranger_Protocol_mngr_req_RgrMngId_h
#define swc_ranger_Protocol_mngr_req_RgrMngId_h

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RgrMngId.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


class RgrMngId: public client::ConnQueue::ReqBase {

  public:
  typedef std::shared_ptr<RgrMngId> Ptr;

  RgrMngId(asio::io_context* io, const std::function<void()>& cb = 0) 
          : client::ConnQueue::ReqBase(false, nullptr),
            cfg_check_interval(
              Env::Config::settings()->get<Property::V_GINT32>(
                "swc.rgr.id.validation.interval")), 
            cb_shutdown(cb),
            m_timer(asio::high_resolution_timer(*io)),
            m_run(true) {
  }
  
  virtual ~RgrMngId() { }

  void create(const Params::RgrMngId& params) {
    std::lock_guard lock(m_mutex);
    cbp = CommBuf::make(params);
    cbp->header.set(RGR_MNG_ID, 60000);
  }

  void request() {
    cancel();

    if(RangerEnv::is_shuttingdown()) {
      auto rgr_data = RangerEnv::rgr_data();
      SWC_LOGF(LOG_DEBUG, "RS_SHUTTINGDOWN(req) %s",  
               rgr_data->to_string().c_str());
      create(
        Params::RgrMngId(
          rgr_data->rgrid.load(), 
          Params::RgrMngId::Flag::RS_SHUTTINGDOWN, 
          rgr_data->endpoints
        )
      );

    } else {
      create(
        Params::RgrMngId(
          0, 
          Params::RgrMngId::Flag::RS_REQ, 
          RangerEnv::rgr_data()->endpoints
        )
      );
    }

    run();
  }

  void handle_no_conn() override {
    clear_endpoints();
    set(200);
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()) {
      Env::Clients::get()->mngrs_groups->select(
        Types::MngrRole::RANGERS, endpoints);
      if(endpoints.empty()) {
        MngrActive::make(Types::MngrRole::RANGERS, shared_from_this())->run();
        return false;
      }
    }
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {

    if(ev->error != Error::OK || ev->header.command != RGR_MNG_ID) {
      set(1000);
      return;
    }

    if(ev->response_code() == Error::OK) {
      set(0);
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
        
    if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_REREQ) {
      request();
      return;
    }
    
    if(rsp_params.flag == Params::RgrMngId::Flag::RS_SHUTTINGDOWN) {
      SWC_LOGF(LOG_DEBUG, "RS_SHUTTINGDOWN %s", 
                RangerEnv::rgr_data()->to_string().c_str());
      stop();
      if(cb_shutdown)
        cb_shutdown();
      else
        SWC_LOG(LOG_WARN, "Shutdown flag without Callback!");
      return;
    }

    if(rsp_params.flag != Params::RgrMngId::Flag::MNGR_ASSIGNED
        &&
       rsp_params.flag != Params::RgrMngId::Flag::MNGR_REASSIGN) {
      clear_endpoints();
      // remain Flag can be only MNGR_NOT_ACTIVE || no other action 
      set(1000);
      return;
    }

    auto rgr_data = RangerEnv::rgr_data();

    if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_ASSIGNED
       && rsp_params.fs != Env::FsInterface::interface()->get_type()){

      SWC_LOGF(LOG_ERROR, "Ranger's %s not matching with Mngr's FS-type=%d,"
                      "RS_SHUTTINGDOWN %s",
        Env::FsInterface::interface()->to_string().c_str(), 
        (int)rsp_params.fs, 
        rgr_data->to_string().c_str()
      );
        
      std::raise(SIGTERM);
      return;
    }
    
    Params::RgrMngId::Flag flag;
    if(!rgr_data->rgrid || rgr_data->rgrid == rsp_params.rgrid || 
       (rgr_data->rgrid != rsp_params.rgrid && 
        rsp_params.flag == Params::RgrMngId::Flag::MNGR_REASSIGN)){

      rgr_data->rgrid = rsp_params.rgrid;
      flag = Params::RgrMngId::Flag::RS_ACK;
      SWC_LOGF(LOG_DEBUG, "RS_ACK %s", rgr_data->to_string().c_str());
    } else {

      flag = Params::RgrMngId::Flag::RS_DISAGREE;
      SWC_LOGF(LOG_DEBUG, "RS_DISAGREE %s", rgr_data->to_string().c_str());
    }

    create(Params::RgrMngId(rgr_data->rgrid, flag, rgr_data->endpoints));
    run();
  }
  
  private:

  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  void stop() {
    std::lock_guard lock(m_mutex);
    if(m_run) 
      m_timer.cancel();
    m_run = false;
  }

  void set(uint32_t ms) {
    cancel();

    std::lock_guard lock(m_mutex);
    if(!m_run)
      return;

    m_timer.expires_from_now(
      std::chrono::milliseconds(ms ? ms : cfg_check_interval->get()));

    m_timer.async_wait(
      [this](const asio::error_code ec) {
        if(ec != asio::error::operation_aborted)
          request();
      }
    );
  }

  void cancel() {
    std::lock_guard lock(m_mutex);
    m_timer.cancel();
  }

  const Property::V_GINT32::Ptr cfg_check_interval;
  const std::function<void()>   cb_shutdown;
  EndPoints                     endpoints;
  
  std::mutex                    m_mutex;
  asio::high_resolution_timer   m_timer;
  bool                          m_run;

};

}}}}

#endif // swc_ranger_Protocol_mngr_req_RgrMngId_h

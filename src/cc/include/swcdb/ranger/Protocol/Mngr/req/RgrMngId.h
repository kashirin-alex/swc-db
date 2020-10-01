
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_Protocol_mngr_req_RgrMngId_h
#define swcdb_ranger_Protocol_mngr_req_RgrMngId_h

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RgrMngId.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class RgrMngId: public client::ConnQueue::ReqBase {

  public:
  typedef std::shared_ptr<RgrMngId> Ptr;

  RgrMngId(asio::io_context* io, const std::function<void()>& cb = 0) 
          : client::ConnQueue::ReqBase(false, nullptr),
            cfg_check_interval(
              Env::Config::settings()->get<Config::Property::V_GINT32>(
                "swc.rgr.id.validation.interval")), 
            cb_shutdown(cb),
            m_timer(asio::high_resolution_timer(*io)),
            m_run(true), m_failures(0) {
  }
  
  virtual ~RgrMngId() { }

  void create(const Params::RgrMngId& params) {
    std::lock_guard lock(m_mutex);
    cbp = Buffers::make(params);
    cbp->header.set(RGR_MNG_ID, 60000);
  }

  void request() {
    cancel();

    auto rgr_data = Env::Rgr::rgr_data();
    if(Env::Rgr::is_shuttingdown()) {
      SWC_LOG_OUT(LOG_DEBUG,
        rgr_data->print(SWC_LOG_OSTREAM << "RS_SHUTTINGDOWN(req) "); );
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
          rgr_data->endpoints
        )
      );
    }

    run();
  }

  void handle_no_conn() override {
    clear_endpoints();
    set(200);
  }

  bool run() override {
    if(endpoints.empty()) {
      Env::Clients::get()->mngrs_groups->select(
        DB::Types::MngrRole::RANGERS, endpoints);
      if(endpoints.empty()) {
        MngrActive::make(
          DB::Types::MngrRole::RANGERS, shared_from_this())->run();
        return false;
      }
    }
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {

    if(ev->error) {
      set(1000);
      /* 
      ++m_failures >? #3
        m_failures = 0;
        > all ranges:
          > LastRgr this
            > UnloadRange cb
              > req Mngr RangeUnloaded
          > LastRgr not this
            > UnloadRange cb
              > req LastRgr UnloadRangeAvoidDup 
                > LastRgr req Mngr RangeUnloaded
      */
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

    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      set(500);
      return;
    }
        
    if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_REREQ) {
      request();
      return;
    }
    
    if(rsp_params.flag == Params::RgrMngId::Flag::RS_SHUTTINGDOWN) {
      SWC_LOG_OUT(LOG_DEBUG,
        Env::Rgr::rgr_data()->print(SWC_LOG_OSTREAM << "RS_SHUTTINGDOWN ");
      );
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

    auto rgr_data = Env::Rgr::rgr_data();

    if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_ASSIGNED
       && rsp_params.fs != Env::FsInterface::interface()->get_type()) {

      SWC_LOG_OUT(LOG_ERROR,
        SWC_LOG_OSTREAM 
          << "Ranger's " << Env::FsInterface::interface()->to_string()
          << " not matching with Mngr's FS-type=" 
          << FS::to_string(rsp_params.fs);
        rgr_data->print(SWC_LOG_OSTREAM << ", RS_SHUTTINGDOWN ");
      );
        
      std::raise(SIGTERM);
      return;
    }
    
    Params::RgrMngId::Flag flag;
    if(!rgr_data->rgrid || rgr_data->rgrid == rsp_params.rgrid || 
       (rgr_data->rgrid != rsp_params.rgrid && 
        rsp_params.flag == Params::RgrMngId::Flag::MNGR_REASSIGN)) {

      rgr_data->rgrid = rsp_params.rgrid;
      flag = Params::RgrMngId::Flag::RS_ACK;
      SWC_LOG_OUT(LOG_DEBUG,
        rgr_data->print(SWC_LOG_OSTREAM << "RS_ACK "); );
    } else {

      flag = Params::RgrMngId::Flag::RS_DISAGREE;
      SWC_LOG_OUT(LOG_DEBUG, 
        rgr_data->print(SWC_LOG_OSTREAM << "RS_DISAGREE "); );
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

    m_timer.expires_after(
      std::chrono::milliseconds(ms ? ms : cfg_check_interval->get()));

    m_timer.async_wait(
      [this](const asio::error_code& ec) {
        if(ec != asio::error::operation_aborted)
          request();
      }
    );
  }

  void cancel() {
    std::lock_guard lock(m_mutex);
    m_timer.cancel();
  }

  const Config::Property::V_GINT32::Ptr cfg_check_interval;
  const std::function<void()>   cb_shutdown;
  EndPoints                     endpoints;
  
  std::mutex                    m_mutex;
  asio::high_resolution_timer   m_timer;
  bool                          m_run;
  std::atomic<size_t>           m_failures;

};

}}}}}

#endif // swcdb_ranger_Protocol_mngr_req_RgrMngId_h

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

  RgrMngId(const IoContextPtr& ioctx,
           std::function<void()>&& cb = nullptr)
          : client::ConnQueue::ReqBase(nullptr),
            cfg_check_interval(
              Env::Config::settings()->get<Config::Property::V_GINT32>(
                "swc.rgr.id.validation.interval")),
            cb_shutdown(std::move(cb)),
            m_timer(asio::high_resolution_timer(ioctx->executor())),
            m_run(true), m_failures(0) {
  }

  virtual ~RgrMngId() { }

  void create(const Params::RgrMngId& params) {
    Core::MutexAtomic::scope lock(m_mutex);
    cbp = Buffers::make(params, 0, RGR_MNG_ID, 60000);
  }

  void request() {
    cancel();

    auto rgr_data = Env::Rgr::rgr_data();
    if(Env::Rgr::is_shuttingdown()) {
      SWC_LOG_OUT(LOG_DEBUG,
        rgr_data->print(SWC_LOG_OSTREAM << "RGR_SHUTTINGDOWN(req) "); );
      create(
        Params::RgrMngId(
          rgr_data->rgrid.load(),
          Params::RgrMngId::Flag::RGR_SHUTTINGDOWN,
          rgr_data->endpoints
        )
      );

    } else if(Env::Rgr::is_not_accepting()) {
      return set(0);

    } else {
      create(
        Params::RgrMngId(
          0,
          Params::RgrMngId::Flag::RGR_REQ,
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
      Env::Clients::get()->get_mngr(DB::Types::MngrRole::RANGERS, endpoints);
      if(endpoints.empty()) {
        MngrActive::make(
          Env::Clients::get(),
          DB::Types::MngrRole::RANGERS, shared_from_this())->run();
        return false;
      }
    }
    Env::Clients::get()->get_mngr_queue(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {

    if(ev->error || ev->response_code() != Error::OK) {
      return set(1000);
    }

    Params::RgrMngId rsp_params;
    try {
      const uint8_t *ptr = ev->data.base + 4;
      size_t remain = ev->data.size - 4;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      return set(500);
    }

    switch(rsp_params.flag) {

      case Params::RgrMngId::Flag::MNGR_ACK: {
        return set(0);
      }

      case Params::RgrMngId::Flag::MNGR_REREQ: {
        return request();
      }

      case Params::RgrMngId::Flag::RGR_SHUTTINGDOWN: {
        SWC_LOG_OUT(LOG_DEBUG,
          Env::Rgr::rgr_data()->print(SWC_LOG_OSTREAM << "RGR_SHUTTINGDOWN ");
        );
        stop();
        if(cb_shutdown)
          cb_shutdown();
        else
          SWC_LOG(LOG_WARN, "Shutdown flag without Callback!");
        return;
      }

      case Params::RgrMngId::Flag::MNGR_REASSIGN:
      case Params::RgrMngId::Flag::MNGR_ASSIGNED: {

        auto rgr_data = Env::Rgr::rgr_data();

        if(rsp_params.flag == Params::RgrMngId::Flag::MNGR_ASSIGNED &&
           rsp_params.fs
            != Env::FsInterface::interface()->get_type_underlying()) {

          SWC_LOG_OUT(LOG_ERROR,
            SWC_LOG_OSTREAM
              << "Ranger's " << Env::FsInterface::interface()->to_string()
              << " not matching with Mngr's FS-type="
              << FS::to_string(rsp_params.fs);
            rgr_data->print(SWC_LOG_OSTREAM << ", RGR_SHUTTINGDOWN ");
          );

          std::raise(SIGTERM);
          return;
        }

        Params::RgrMngId::Flag flag;
        if(!rgr_data->rgrid || rgr_data->rgrid == rsp_params.rgrid ||
           (rgr_data->rgrid != rsp_params.rgrid &&
           rsp_params.flag == Params::RgrMngId::Flag::MNGR_REASSIGN)) {

          rgr_data->rgrid.store(rsp_params.rgrid);
          flag = Params::RgrMngId::Flag::RGR_ACK;
          SWC_LOG_OUT(LOG_DEBUG,
            rgr_data->print(SWC_LOG_OSTREAM << "RGR_ACK "); );
        } else {

          flag = Params::RgrMngId::Flag::RGR_DISAGREE;
          SWC_LOG_OUT(LOG_DEBUG,
            rgr_data->print(SWC_LOG_OSTREAM << "RGR_DISAGREE "); );
        }

        create(Params::RgrMngId(rgr_data->rgrid, flag, rgr_data->endpoints));
        run();
        return;
      }

      default: {
        clear_endpoints();
        // remain Flag can be only MNGR_NOT_ACTIVE || no other action
        return set(1000);
      }
    }
  }

  private:

  void clear_endpoints() {
    Env::Clients::get()->remove_mngr(endpoints);
    endpoints.clear();
  }

  void stop() {
    bool at = true;
    if(m_run.compare_exchange_weak(at, false)) {
      Core::MutexAtomic::scope lock(m_mutex);
      m_timer.cancel();
    }
  }

  void set(uint32_t ms) {
    cancel();

    Core::MutexAtomic::scope lock(m_mutex);
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
    Core::MutexAtomic::scope lock(m_mutex);
    m_timer.cancel();
  }

  const Config::Property::V_GINT32::Ptr cfg_check_interval;
  const std::function<void()>   cb_shutdown;
  EndPoints                     endpoints;

  Core::MutexAtomic             m_mutex;
  asio::high_resolution_timer   m_timer;
  Core::AtomicBool              m_run;
  Core::Atomic<size_t>          m_failures;

};

}}}}}

#endif // swcdb_ranger_Protocol_mngr_req_RgrMngId_h

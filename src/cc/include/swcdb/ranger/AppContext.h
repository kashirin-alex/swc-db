/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_AppContext_h
#define swcdb_ranger_AppContext_h


#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"
#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/core/comm/DispatchHandler.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/ranger/RangerEnv.h"

#include "swcdb/common/Protocol/handlers/NotImplemented.h"
#include "swcdb/ranger/Protocol/Mngr/req/RgrMngId.h"

#include "swcdb/ranger/Protocol/handlers/AssignId.h"
#include "swcdb/ranger/Protocol/handlers/RangeLoad.h"
#include "swcdb/ranger/Protocol/handlers/RangeUnload.h"
#include "swcdb/ranger/Protocol/handlers/RangeIsLoaded.h"
#include "swcdb/ranger/Protocol/handlers/RangeLocate.h"
#include "swcdb/ranger/Protocol/handlers/RangeQueryUpdate.h"
#include "swcdb/ranger/Protocol/handlers/RangeQuerySelect.h"
#include "swcdb/ranger/Protocol/handlers/ColumnUpdate.h"
#include "swcdb/ranger/Protocol/handlers/ColumnDelete.h"
#include "swcdb/ranger/Protocol/handlers/ColumnCompact.h"
#include "swcdb/ranger/Protocol/handlers/Report.h"
#include "swcdb/ranger/Protocol/handlers/ColumnsUnload.h"


namespace SWC { namespace Ranger {



class AppContext final : public Comm::AppContext {
  public:
  typedef std::shared_ptr<AppContext> Ptr;

  SWC_SHOULD_NOT_INLINE
  static Ptr make() {
    auto settings = Env::Config::settings();

    settings->parse_file(
      settings->get_str("swc.rgr.cfg", ""),
      "swc.rgr.cfg.dyn"
    );

    Env::Clients::init(
      client::Clients::make(
        *settings,
        Comm::IoContext::make(
          "Clients",
           settings->get_i32("swc.rgr.clients.handlers")
        ),
        client::ContextManager::Ptr(
          new client::ContextManager(*settings)),
        client::ContextRanger::Ptr(
          new client::ContextRanger(*settings)),
        client::ContextBroker::Ptr(
          new client::ContextBroker(*settings))
      )->init()
    );

    Env::FsInterface::init(
      settings,
      FS::fs_type(settings->get_str("swc.fs"))
    );

    Env::Rgr::init();

    auto period = settings->get<Config::Property::Value_int32_g>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::Rgr::io()->set_periodic_timer(
        period,
        []() noexcept { Env::Config::settings()->check_dynamic_files(); }
      );
    }

    Ptr app(new AppContext());
    app->id_mngr.reset(new Comm::Protocol::Mngr::Req::RgrMngId(
      Env::Rgr::io(),
      [app]() {
        std::shared_ptr<std::thread> d(new std::thread);
        *d.get() = std::thread([d, app]{ app->stop(); });
        d->detach();
      }
    ));
    return app;
  }

  AppContext()
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::Value_enum_g>(
            "swc.rgr.comm.encoder")),
        m_metrics(Env::Rgr::metrics_track()) {
  }

  void init(const std::string& host,
            const Comm::EndPoints& endpoints) override {
    Env::Rgr::rgr_data()->endpoints = endpoints;

    int sig = 0;
    Env::Rgr::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Rgr::start();
    id_mngr->request();

    if(m_metrics) {
      m_metrics->configure_rgr(host.c_str(), endpoints);
      m_metrics->start();
    }
  }

  void set_srv(Comm::server::SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext() noexcept { }

  void handle_established(Comm::ConnHandlerPtr conn) override {
    m_srv->connection_add(conn);
    if(m_metrics)
      m_metrics->net->connected(conn);
  }

  void handle_disconnect(Comm::ConnHandlerPtr conn) noexcept override {
    m_srv->connection_del(conn);
    if(m_metrics)
      m_metrics->net->disconnected(conn);
  }

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    // SWC_LOG_OUT(LOG_DEBUG, ev->print(SWC_LOG_OSTREAM << "handle: "); );

    #if defined(SWC_ENABLE_SANITIZER)
      if(Env::Rgr::is_shuttingdown())
        return conn->do_close();
    #endif

    if(ev->error) {
      m_metrics->net->error(conn);
      return;
    }

    switch(Env::Rgr::rgr_data()->rgrid
            ? Comm::Protocol::Rgr::Command(ev->header.command)
            : Comm::Protocol::Rgr::Command::MAX_CMD) {

      case Comm::Protocol::Rgr::Command::MAX_CMD:
        conn->send_error(Error::RGR_NOT_READY, "", ev);
        break;

      case Comm::Protocol::Rgr::Command::ASSIGN_ID_NEEDED: // not-used
        Comm::Protocol::Rgr::Handler::assign_id(conn, ev, id_mngr);
        break;

      case Comm::Protocol::Rgr::Command::COLUMN_DELETE:
        Comm::Protocol::Rgr::Handler::column_delete(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::COLUMN_COMPACT:
        Comm::Protocol::Rgr::Handler::column_compact(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::SCHEMA_UPDATE:
        Comm::Protocol::Rgr::Handler::column_update(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::COLUMNS_UNLOAD:
        Comm::Protocol::Rgr::Handler::columns_unload(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::RANGE_IS_LOADED:
        Comm::Protocol::Rgr::Handler::range_is_loaded(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::RANGE_LOAD:
        Env::Rgr::post(
          Comm::Protocol::Rgr::Handler::RangeLoad(conn, ev));
        break;

      case Comm::Protocol::Rgr::Command::RANGE_UNLOAD:
        Comm::Protocol::Rgr::Handler::range_unload(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::RANGE_LOCATE:
        Env::Rgr::post(
          Comm::Protocol::Rgr::Handler::RangeLocate(conn, ev));
        break;

      case Comm::Protocol::Rgr::Command::RANGE_QUERY_UPDATE:
        Comm::Protocol::Rgr::Handler::range_query_update(conn, ev);
        break;

      case Comm::Protocol::Rgr::Command::RANGE_QUERY_SELECT:
        Env::Rgr::post(
          Comm::Protocol::Rgr::Handler::RangeQuerySelect(conn, ev));
        break;

      case Comm::Protocol::Rgr::Command::REPORT:
        Env::Rgr::post(
          Comm::Protocol::Rgr::Handler::Report(conn, ev));
        break;

      //&Comm::Protocol::Rgr::Handler::debug,
      //&Comm::Protocol::Rgr::Handler::status,
      /*
      case Comm::Protocol::Rgr::Handler::shutdown: {
        struct Task {
          AppContext* ptr;
          SWC_CAN_INLINE
          Task(AppContext* a_ptr) noexcept : ptr(a_ptr) { }
          void operator()() { ptr->shutting_down(std::error_code(), SIGINT); }
        };
        Env::Rgr::post(Task(this));
        conn->response_ok(ev);
        break;
      }
      */

      default: {
        Comm::Protocol::Common::Handler::not_implemented(conn, ev);
        if(m_metrics)
          m_metrics->net->error(conn);
        return;
      }
    }

    if(m_metrics)
      m_metrics->net->command(conn, ev->header.command);
  }

  void net_bytes_sent(const Comm::ConnHandlerPtr& conn, size_t b)
                      noexcept override {
    if(m_metrics)
      m_metrics->net->sent(conn, b);
  }

  void net_bytes_received(const Comm::ConnHandlerPtr& conn, size_t b)
                          noexcept override {
    if(m_metrics)
      m_metrics->net->received(conn, b);
  }

  void net_accepted(const Comm::EndPoint& endpoint, bool secure)
                    noexcept override {
    if(m_metrics)
      m_metrics->net->accepted(endpoint, secure);
  }

  void shutting_down(const std::error_code &ec, const int &sig) {
    if(!sig) { // set signals listener
      struct Task {
        AppContext* ptr;
        SWC_CAN_INLINE
        Task(AppContext* a_ptr) noexcept : ptr(a_ptr) { }
        void operator()(const std::error_code& ec, const int &sig) {
          if(ec == asio::error::operation_aborted)
            return;
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s",
                    sig, ec.message().c_str());
          ptr->shutting_down(ec, sig);
        }
      };
      Env::Rgr::io()->signals->async_wait(Task(this));

      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s",
              sig, ec.message().c_str());
      return;
    }
    Env::Rgr::io()->signals->cancel();
    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s",
             sig, ec.message().c_str());

    if(!m_srv) {
      SWC_LOG(LOG_INFO, "Exit");
      SWC_QUICK_EXIT(EXIT_SUCCESS);
    }


    Env::Rgr::shuttingdown();

    m_guard = m_srv->stop_accepting(); // no further requests accepted

    id_mngr->request();
  }

  void stop() override {
    Env::Rgr::wait_if_in_process();

    #if defined(SWC_ENABLE_SANITIZER)
      if(m_metrics)
        m_metrics->wait();
    #endif

    Env::Clients::get()->stop();

    Env::FsInterface::interface()->stop();

    Env::Rgr::io()->stop();

    m_srv->shutdown();

    #if defined(SWC_ENABLE_SANITIZER)
      std::this_thread::sleep_for(std::chrono::seconds(2));
      m_metrics = nullptr;
      id_mngr = nullptr;
      m_srv = nullptr;
      Env::Rgr::reset();
      Env::Clients::reset();
      Env::FsInterface::reset();
    #endif

    m_guard = nullptr;
  }

  private:

  Comm::Protocol::Mngr::Req::RgrMngId::Ptr  id_mngr = nullptr;
  Comm::server::SerializedServer::Ptr       m_srv = nullptr;
  Metric::Reporting::Ptr                    m_metrics = nullptr;
  std::shared_ptr<Comm::IoContext::ExecutorWorkGuard> m_guard;

};


}}



#endif // swcdb_ranger_AppContext_h

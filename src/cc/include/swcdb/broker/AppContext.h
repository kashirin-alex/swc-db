/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_broker_AppContext_h
#define swcdb_broker_AppContext_h


#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"
#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/core/comm/DispatchHandler.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/broker/BrokerEnv.h"

#include "swcdb/common/Protocol/handlers/NotImplemented.h"
#include "swcdb/broker/Protocol/handlers/ColumnGet.h"
#include "swcdb/broker/Protocol/handlers/ColumnList.h"
#include "swcdb/broker/Protocol/handlers/ColumnCompact.h"
#include "swcdb/broker/Protocol/handlers/ColumnMng.h"
#include "swcdb/broker/Protocol/handlers/CellsUpdate.h"
#include "swcdb/broker/Protocol/handlers/CellsSelect.h"


namespace SWC { namespace Broker {



class AppContext final : public Comm::AppContext {

  struct CommandHandler {
    // in-order of Protocol::Bkr::Command
    static constexpr const Comm::AppHandler_t handlers[] = {
      &Comm::Protocol::Common::Handler::not_implemented,
      &Comm::Protocol::Bkr::Handler::column_get,
      &Comm::Protocol::Bkr::Handler::column_list,
      &Comm::Protocol::Bkr::Handler::column_compact,
      &Comm::Protocol::Bkr::Handler::column_mng,
      &Comm::Protocol::Bkr::Handler::cells_update,
      &Comm::Protocol::Bkr::Handler::cells_select,
    };
    Comm::ConnHandlerPtr conn;
    Comm::Event::Ptr     ev;
    SWC_CAN_INLINE
    CommandHandler(const Comm::ConnHandlerPtr& a_conn,
                   const Comm::Event::Ptr& a_ev) noexcept
                : conn(a_conn), ev(a_ev) {
      Env::Bkr::in_process(1);
    }
    ~CommandHandler() noexcept { }
    void operator()() {
      ev->expired() || !conn->is_open()
        ? Env::Bkr::processed()
        : handlers[ev->header.command](conn, ev);
    }
  };


  public:

  typedef std::shared_ptr<AppContext> Ptr;

  static Ptr make() {
    auto settings = Env::Config::settings();

    settings->parse_file(
      settings->get_str("swc.bkr.cfg", ""),
      "swc.bkr.cfg.dyn"
    );

    Env::Clients::init(
      client::Clients::make(
        *settings,
        Comm::IoContext::make(
          "Clients",
          settings->get_i32("swc.bkr.clients.handlers")
        ),
        client::ContextManager::Ptr(
          new client::ContextManager(*settings)),
        client::ContextRanger::Ptr(
          new client::ContextRanger(*settings))
      )->init()
    );

    Env::Bkr::init();

    auto period = settings->get<Config::Property::V_GINT32>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::Bkr::io()->set_periodic_timer(
        period,
        [](){Env::Config::settings()->check_dynamic_files();}
      );
    }

    return Ptr(new AppContext());
  }

  AppContext()
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::V_GENUM>(
            "swc.bkr.comm.encoder")),
        m_metrics(Env::Bkr::metrics_track()) {
  }

  void init(const std::string& host,
            const Comm::EndPoints& endpoints) override {

    int sig = 0;
    Env::Bkr::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Bkr::start();

    if(m_metrics) {
      m_metrics->configure_bkr(host.c_str(), endpoints);
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

    if(!Env::Bkr::can_process())
      return conn->do_close();

    if(ev->error) {
      m_metrics->net->error(conn);

    } else if(!ev->header.command ||
               ev->header.command >= Comm::Protocol::Bkr::MAX_CMD) {
      Comm::Protocol::Common::Handler::not_implemented(conn, ev);
      if(m_metrics)
        m_metrics->net->error(conn);

    } else {
      Env::Bkr::post(CommandHandler(conn, ev));
      if(m_metrics)
        m_metrics->net->command(conn, ev->header.command);
    }

    Env::Bkr::processed();
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
      Env::Bkr::io()->signals->async_wait(Task(this));

      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s",
              sig, ec.message().c_str());
      return;
    }
    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s",
             sig, ec.message().c_str());

    if(!m_srv) {
      SWC_LOG(LOG_INFO, "Exit");
      SWC_QUICK_EXIT(EXIT_SUCCESS);
    }

    std::shared_ptr<std::thread> d(new std::thread);
    *d.get() = std::thread([d, ptr=shared_from_this()]{ ptr->stop(); });
    d->detach();
  }

  void stop() override {
    auto guard = m_srv->stop_accepting(); // no further requests accepted

    Env::Bkr::shuttingdown();

    #if defined(SWC_ENABLE_SANITIZER)
      if(m_metrics)
        m_metrics->wait();
    #endif

    Env::Clients::get()->stop();

    Env::Bkr::io()->stop();

    m_srv->shutdown();

    #if defined(SWC_ENABLE_SANITIZER)
      std::this_thread::sleep_for(std::chrono::seconds(2));
      m_metrics = nullptr;
      m_srv = nullptr;
      Env::Bkr::reset();
      Env::Clients::reset();
    #endif

    guard = nullptr;
  }

  private:

  Comm::server::SerializedServer::Ptr       m_srv = nullptr;
  Metric::Reporting::Ptr                    m_metrics = nullptr;

};


}}



#endif // swcdb_broker_AppContext_h

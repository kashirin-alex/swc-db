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
/*
#include "swcdb/broker/Protocol/handlers/column_get.h"
#include "swcdb/broker/Protocol/handlers/column_list.h"
#include "swcdb/broker/Protocol/handlers/column_compact.h"
#include "swcdb/broker/Protocol/handlers/column_mng.h"
#include "swcdb/broker/Protocol/handlers/cells_update.h"
#include "swcdb/broker/Protocol/handlers/cells_select.h"
*/

namespace SWC { namespace Broker {



class AppContext final : public Comm::AppContext {
  public:

  static std::shared_ptr<AppContext> make() {
    auto settings = Env::Config::settings();

    settings->parse_file(
      settings->get_str("swc.bkr.cfg", ""),
      "swc.bkr.cfg.dyn"
    );

    Env::IoCtx::init(settings->get_i32("swc.bkr.clients.handlers"));
    Env::Clients::init(
      std::make_shared<client::Clients>(
        *settings,
        Env::IoCtx::io(),
        std::make_shared<client::ContextManager>(),
        std::make_shared<client::ContextRanger>()
      )
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

    auto app = std::make_shared<AppContext>();
    return app;
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

  virtual ~AppContext() { }


  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    // SWC_LOG_OUT(LOG_DEBUG, ev->print(SWC_LOG_OSTREAM << "handle: "); );

    #if defined(SWC_ENABLE_SANITIZER)
      if(Env::Bkr::is_shuttingdown())
        return conn->do_close();
    #endif

    switch (ev->type) {

      case Comm::Event::Type::ESTABLISHED:
        m_srv->connection_add(conn);
        if(m_metrics)
          m_metrics->net->connected(conn);
        break;

      case Comm::Event::Type::DISCONNECT:
        m_srv->connection_del(conn);
        if(m_metrics)
          m_metrics->net->disconnected(conn);
        break;

      case Comm::Event::Type::ERROR:
        if(m_metrics)
          m_metrics->net->error(conn);
        break;

      case Comm::Event::Type::MESSAGE: {
        Env::Bkr::in_process(1);
        switch(ev->header.command) {
          /*
          case Comm::Protocol::Bkr::Command::COLUMN_GET:
            Env::Bkr::post([conn, ev]() {
              if(!ev->expired())
                Comm::Protocol::Bkr::Handler::column_get(conn, ev);
            });
            break;

          case Comm::Protocol::Bkr::Command::COLUMN_LIST:
            Env::Bkr::post([conn, ev]() {
              if(!ev->expired())
                Comm::Protocol::Bkr::Handler::column_list(conn, ev);
            });
            break;

          case Comm::Protocol::Bkr::Command::COLUMN_COMPACT:
            Env::Bkr::post([conn, ev]() {
              if(!ev->expired())
                Comm::Protocol::Bkr::Handler::column_compact(conn, ev);
            });
            break;

          case Comm::Protocol::Bkr::Command::COLUMN_MNG:
            Env::Bkr::post([conn, ev]() {
              if(!ev->expired())
                Comm::Protocol::Bkr::Handler::column_mng(conn, ev);
            });
            break;

          case Comm::Protocol::Bkr::Command::CELLS_UPDATE:
            Env::Bkr::post([conn, ev]() {
              if(!ev->expired())
                Comm::Protocol::Bkr::Handler::cells_update(conn, ev);
            });
            break;

          case Comm::Protocol::Bkr::Command::CELLS_SELECT:
            Env::Bkr::post([conn, ev]() {
              if(!ev->expired())
                Comm::Protocol::Bkr::Handler::cells_select(conn, ev);
            });
            break;
          */
          default:
            Comm::Protocol::Common::Handler::not_implemented(conn, ev);
            if(m_metrics)
              m_metrics->net->error(conn);
            return;
        }
        if(m_metrics)
          m_metrics->net->command(conn, ev->header.command);
        break;
      }

      default: {
        SWC_LOGF(LOG_WARN, "Unimplemented event-type (%d)", int(ev->type));
        if(m_metrics)
          m_metrics->net->error(conn);
        break;
      }
    }
  }

  void net_bytes_sent(const Comm::ConnHandlerPtr& conn, size_t b) override {
    if(m_metrics)
      m_metrics->net->sent(conn, b);
  }

  void net_bytes_received(const Comm::ConnHandlerPtr& conn, size_t b) override {
    if(m_metrics)
      m_metrics->net->received(conn, b);
  }

  void accepted(const Comm::EndPoint& endpoint, bool secure) override {
    if(m_metrics)
      m_metrics->net->accepted(endpoint, secure);
  }

  void shutting_down(const std::error_code &ec, const int &sig) {
    if(!sig) { // set signals listener
      Env::Bkr::io()->signals->async_wait(
        [this](const std::error_code &ec, const int &sig) {
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s",
                   sig, ec.message().c_str());
          shutting_down(ec, sig);
        }
      );
      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s",
              sig, ec.message().c_str());
      return;
    }
    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s",
             sig, ec.message().c_str());

    if(!m_srv) {
      SWC_LOG(LOG_INFO, "Exit");
      std::quick_exit(EXIT_SUCCESS);
    }

    std::shared_ptr<std::thread> d(new std::thread);
    *d.get() = std::thread([d, ptr=shared_from_this()]{ ptr->stop(); });
    d->detach();
  }

  void stop() override {
    auto guard = m_srv->stop_accepting(); // no further requests accepted

    Env::Bkr::shuttingdown();

    Env::Clients::get()->stop();
    Env::IoCtx::io()->stop();

    Env::Bkr::io()->stop();

    m_srv->shutdown();

    #if defined(SWC_ENABLE_SANITIZER)
      std::this_thread::sleep_for(std::chrono::seconds(2));
      if(m_metrics) {
        m_metrics->wait();
        m_metrics = nullptr;
      }
      m_srv = nullptr;
      Env::Bkr::reset();
      Env::Clients::reset();
      Env::IoCtx::reset();
    #endif

    guard = nullptr;
  }

  private:

  Comm::server::SerializedServer::Ptr       m_srv = nullptr;
  Metric::Reporting::Ptr                    m_metrics = nullptr;

};


}}



#endif // swcdb_broker_AppContext_h

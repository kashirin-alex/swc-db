/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_AppContext_h
#define swcdb_manager_AppContext_h


#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"
#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/core/comm/DispatchHandler.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Clients.h"

#include "swcdb/manager/ClientContextManager.h"
#include "swcdb/manager/MngrEnv.h"

#include "swcdb/common/Protocol/handlers/NotImplemented.h"
#include "swcdb/manager/Protocol/handlers/MngrState.h"
#include "swcdb/manager/Protocol/handlers/MngrActive.h"
#include "swcdb/manager/Protocol/handlers/ColumnMng.h"
#include "swcdb/manager/Protocol/handlers/ColumnUpdate.h"
#include "swcdb/manager/Protocol/handlers/ColumnGet.h"
#include "swcdb/manager/Protocol/handlers/ColumnList.h"
#include "swcdb/manager/Protocol/handlers/ColumnCompact.h"
#include "swcdb/manager/Protocol/handlers/RgrMngId.h"
#include "swcdb/manager/Protocol/handlers/RgrUpdate.h"
#include "swcdb/manager/Protocol/handlers/RgrGet.h"
#include "swcdb/manager/Protocol/handlers/RangeCreate.h"
#include "swcdb/manager/Protocol/handlers/RangeUnloaded.h"
#include "swcdb/manager/Protocol/handlers/RangeRemove.h"
#include "swcdb/manager/Protocol/handlers/Report.h"
#include "swcdb/manager/Protocol/handlers/Echo.h"


namespace SWC { namespace Manager {


class AppContext final : public Comm::AppContext {
  public:

  typedef std::shared_ptr<AppContext> Ptr;

  AppContext()
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::V_GENUM>(
            "swc.mngr.comm.encoder")) {
    auto settings = Env::Config::settings();

    settings->parse_file(
      settings->get_str("swc.mngr.cfg", ""),
      "swc.mngr.cfg.dyn"
    );

    Env::Clients::init(
      client::Clients::make(
        *settings,
        Comm::IoContext::make(
          "Clients",
           settings->get_i32("swc.mngr.clients.handlers")
        ),
        client::Mngr::ContextManager::Ptr(
          new client::Mngr::ContextManager()),
        client::ContextRanger::Ptr(
          new client::ContextRanger(*settings)),
        client::ContextBroker::Ptr(
          new client::ContextBroker(*settings))
      )->init()
    );
    Env::Clients::get()->set_flags__schemas_via_default();

    Env::FsInterface::init(
      settings,
      FS::fs_type(settings->get_str("swc.fs"))
    );
  }

  void init(const std::string& host,
            const Comm::EndPoints& endpoints) override {
    Env::Mngr::init(endpoints);

    auto period = Env::Config::settings()->get<Config::Property::V_GINT32>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::Mngr::io()->set_periodic_timer(
        period,
        [](){Env::Config::settings()->check_dynamic_files();}
      );
    }

    int sig = 0;
    Env::Mngr::io()->set_signals();
    shutting_down(std::error_code(), sig);

    if((m_metrics = Env::Mngr::metrics_track())) {
      m_metrics->configure_mngr(host.c_str(), endpoints);
      m_metrics->start();
    }
  }

  void set_srv(Comm::server::SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext() { }

  void handle_established(Comm::ConnHandlerPtr conn) override {
    m_srv->connection_add(conn);
    if(m_metrics)
      m_metrics->net->connected(conn);
  }

  void handle_disconnect(Comm::ConnHandlerPtr conn) noexcept override {
    m_srv->connection_del(conn);
    Env::Mngr::role()->disconnection(
      conn->endpoint_remote, conn->endpoint_local, true);
    if(m_metrics)
      m_metrics->net->disconnected(conn);
  }

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    // SWC_LOG_OUT(LOG_DEBUG, ev->print(SWC_LOG_OSTREAM << "handle: "); );

    #if defined(SWC_ENABLE_SANITIZER)
      if(!Env::Mngr::role()->running())
        return conn->do_close();
    #endif

    if(ev->error) {
      m_metrics->net->error(conn);
      return;
    }

    switch(ev->header.command) {

      case Comm::Protocol::Mngr::Command::MNGR_STATE:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::mngr_state(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::MNGR_ACTIVE:
        Comm::Protocol::Mngr::Handler::mngr_active(conn, ev);
        break;

      case Comm::Protocol::Mngr::Command::COLUMN_MNG:
        Comm::Protocol::Mngr::Handler::column_mng(conn, ev);
            break;

      case Comm::Protocol::Mngr::Command::COLUMN_UPDATE:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::column_update(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::COLUMN_GET:
        Comm::Protocol::Mngr::Handler::column_get(conn, ev);
        break;

      case Comm::Protocol::Mngr::Command::COLUMN_LIST:
        Comm::Protocol::Mngr::Handler::column_list(conn, ev);
        break;

      case Comm::Protocol::Mngr::Command::COLUMN_COMPACT:
        Comm::Protocol::Mngr::Handler::column_compact(conn, ev);
        break;

      case Comm::Protocol::Mngr::Command::RGR_MNG_ID:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::rgr_mng_id(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::RGR_UPDATE:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::rgr_update(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::RGR_GET:
        Comm::Protocol::Mngr::Handler::rgr_get(conn, ev);
        break;

      case Comm::Protocol::Mngr::Command::RANGE_CREATE:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::range_create(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::RANGE_UNLOADED:
        Comm::Protocol::Mngr::Handler::range_unloaded(conn, ev);
        break;

      case Comm::Protocol::Mngr::Command::RANGE_REMOVE:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::range_remove(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::REPORT:
        Env::Mngr::post([conn, ev]() {
          if(!ev->expired())
            Comm::Protocol::Mngr::Handler::report(conn, ev);
        });
        break;

      case Comm::Protocol::Mngr::Command::DO_ECHO:
        Comm::Protocol::Mngr::Handler::do_echo(conn, ev);
        break;

      //&Comm::Protocol::Mngr::Handler::debug,
      //&Comm::Protocol::Mngr::Handler::status,
      //&Comm::Protocol::Mngr::Handler::shutdown

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
      Env::Mngr::io()->signals->async_wait(
        [this](const std::error_code &ec, const int &sig) {
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          shutting_down(ec, sig);
        }
      );
      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s",
              sig, ec.message().c_str());
      return;
    }

    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());

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

    Env::Mngr::stop();

    Env::Clients::get()->stop();

    Env::FsInterface::interface()->stop();

    Env::Mngr::io()->stop();

    m_srv->shutdown();

    #if defined(SWC_ENABLE_SANITIZER)
      std::this_thread::sleep_for(std::chrono::seconds(2));
      if(m_metrics) {
        m_metrics->wait();
        m_metrics = nullptr;
      }
      m_srv = nullptr;
      Env::Mngr::reset();
      Env::Clients::reset();
      Env::FsInterface::reset();
    #endif

    guard = nullptr;
  }

  private:
  Comm::server::SerializedServer::Ptr m_srv = nullptr;
  Metric::Reporting::Ptr              m_metrics = nullptr;

};


}}



#endif // swcdb_manager_AppContext_h

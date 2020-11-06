/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_AppContext_h
#define swcdb_manager_AppContext_h

#include "swcdb/db/Protocol/Commands.h"

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

  AppContext() 
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::V_GENUM>(
            "swc.mngr.comm.encoder")) {
    auto settings = Env::Config::settings();

    settings->parse_file(
      settings->get_str("swc.mngr.cfg", ""), 
      "swc.mngr.cfg.dyn"
    );

    Env::IoCtx::init(settings->get_i32("swc.mngr.clients.handlers"));
    Env::Clients::init(
      std::make_shared<client::Clients>(
        Env::IoCtx::io()->shared(),
        std::make_shared<client::Mngr::ContextManager>(),
        std::make_shared<client::ContextRanger>()
      )
    );

    Env::FsInterface::init(FS::fs_type(settings->get_str("swc.fs")));
  }
  
  void init(const Comm::EndPoints& endpoints) override {
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
  }

  void set_srv(Comm::server::SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext() { }


  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Comm::Event::Type::ESTABLISHED:
        m_srv->connection_add(conn);
        return; 
        
      case Comm::Event::Type::DISCONNECT:
        m_srv->connection_del(conn);
        Env::Mngr::role()->disconnection(
          conn->endpoint_remote, conn->endpoint_local, true);
        return;

      case Comm::Event::Type::ERROR:
        //rangers->decommision(event->addr);
        break;

      case Comm::Event::Type::MESSAGE: {
        switch(ev->header.command) {

          case Comm::Protocol::Mngr::Command::MNGR_STATE:
            Env::Mngr::post([conn, ev]() {
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
              Comm::Protocol::Mngr::Handler::rgr_mng_id(conn, ev);
            });
            break;

          case Comm::Protocol::Mngr::Command::RGR_UPDATE:
            Env::Mngr::post([conn, ev]() {
              Comm::Protocol::Mngr::Handler::rgr_update(conn, ev);
            });
            break;

          case Comm::Protocol::Mngr::Command::RGR_GET:
            Comm::Protocol::Mngr::Handler::rgr_get(conn, ev);
            break;

          case Comm::Protocol::Mngr::Command::RANGE_CREATE:
            Env::Mngr::post([conn, ev]() {
              Comm::Protocol::Mngr::Handler::range_create(conn, ev);
            });
            break;

          case Comm::Protocol::Mngr::Command::RANGE_UNLOADED:
            Comm::Protocol::Mngr::Handler::range_unloaded(conn, ev);
            break;

          case Comm::Protocol::Mngr::Command::RANGE_REMOVE:
            Env::Mngr::post([conn, ev]() {
              Comm::Protocol::Mngr::Handler::range_remove(conn, ev);
            });
            break;

          case Comm::Protocol::Mngr::Command::REPORT:
            Env::Mngr::post([conn, ev]() {
              Comm::Protocol::Mngr::Handler::report(conn, ev);
            });
            break;

          case Comm::Protocol::Mngr::Command::DO_ECHO:
            Comm::Protocol::Mngr::Handler::do_echo(conn, ev);
            break;

          default:
            Comm::Protocol::Common::Handler::not_implemented(conn, ev);
            break;
          //&Comm::Protocol::Mngr::Handler::debug,
          //&Comm::Protocol::Mngr::Handler::status,
          //&Comm::Protocol::Mngr::Handler::shutdown
        }
        break;
      }

      default:
        SWC_LOGF(LOG_WARN, "Unimplemented event-type (%d)", (int)ev->type);
        break;

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig == 0) { // set signals listener
      Env::Mngr::io()->signals()->async_wait(
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
    
    if(m_srv == nullptr) {
      SWC_LOG(LOG_INFO, "Exit");
      std::quick_exit(0);
    }
    
    (new std::thread([ptr=shared_from_this()]{ ptr->stop(); }))->detach();
  }

  void stop() override {
    
    m_srv->stop_accepting(); // no further requests accepted
    
    Env::Mngr::stop();

    Env::Clients::get()->rgr->stop();
    Env::Clients::get()->mngr->stop();
    Env::IoCtx::io()->stop();

    Env::FsInterface::interface()->stop();
    
    Env::Mngr::io()->stop();

    m_srv->shutdown();
    
    SWC_LOG(LOG_INFO, "Exit");
    std::quick_exit(0);
  }

  private:
  Comm::server::SerializedServer::Ptr m_srv = nullptr;
  //ColmNameToIDMap columns;       // column-name > CID


};


}}



#endif // swcdb_manager_AppContext_h

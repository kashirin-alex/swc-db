/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_AppContext_h
#define swc_app_manager_AppContext_h

#include "swcdb/lib/db/Protocol/Commands.h"

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"
#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/client/Clients.h"

#include "swcdb/lib/db/Columns/Schema.h"
#include "swcdb/lib/db/Columns/Mngr/Columns.h"

#include "MngrRole.h"
#include "Rangers.h"

#include "swcdb/lib/db/Protocol/Common/handlers/NotImplemented.h"
#include "swcdb/lib/db/Protocol/Common/handlers/Echo.h"
#include "handlers/MngrState.h"
#include "handlers/MngrActive.h"
#include "handlers/ColumnMng.h"
#include "handlers/ColumnUpdate.h"
#include "handlers/ColumnGet.h"
#include "handlers/RgrMngId.h"
#include "handlers/RgrUpdate.h"
#include "handlers/RgrGet.h"


namespace SWC { namespace server { namespace Mngr {


class AppContext : public SWC::AppContext {
  
  public:

  AppContext() {
    Env::Config::settings()->parse_file(
      Env::Config::settings()->get<String>("swc.mngr.cfg", ""),
      Env::Config::settings()->get<String>("swc.mngr.OnFileChange.cfg", "")
    );

    Env::IoCtx::init(
      Env::Config::settings()->get<int32_t>("swc.mngr.handlers"));
    Env::MngrRole::init();
    Env::FsInterface::init();
    Env::Schemas::init();
    Env::MngrColumns::init();
    Env::Clients::init(std::make_shared<client::Clients>(
      Env::IoCtx::io()->shared(),
      std::make_shared<client::Mngr::AppContext>()
    ));
    Env::Rangers::init();
  }
  
  void init(const EndPoints& endpoints) override {
    Env::MngrRole::get()->init(endpoints);
    
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);
  }

  void set_srv(SerializedServerPtr srv){
    m_srv = srv;
  }

  virtual ~AppContext(){}


  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED:
        m_srv->connection_add(conn);
        return; 
        
      case Event::Type::DISCONNECT:
        m_srv->connection_del(conn);
        if(Env::MngrRole::get()->disconnection(
                          conn->endpoint_remote, conn->endpoint_local, true))
          return;
        return;

      case Event::Type::ERROR:
        //rangers->decommision(event->addr);
        break;

      case Event::Type::MESSAGE: {
        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Mngr::MNGR_ACTIVE:
            handler = new Protocol::Mngr::Handler::MngrActive(conn, ev);
            break;

          case Protocol::Mngr::MNGR_STATE:
            handler = new Protocol::Mngr::Handler::MngrState(conn, ev);
            break;

          case Protocol::Mngr::COLUMN_MNG:
            handler = new Protocol::Mngr::Handler::ColumnMng(conn, ev);
            break;

          case Protocol::Mngr::COLUMN_GET:
            handler = new Protocol::Mngr::Handler::ColumnGet(conn, ev);
            break;

          case Protocol::Mngr::COLUMN_UPDATE:
            handler = new Protocol::Mngr::Handler::ColumnUpdate(conn, ev);
            break;

          case Protocol::Mngr::RGR_GET:
            handler = new Protocol::Mngr::Handler::RgrGet(conn, ev);
            break;

          case Protocol::Mngr::RGR_MNG_ID:
            handler = new Protocol::Mngr::Handler::RgrMngId(conn, ev);
            break;

          case Protocol::Mngr::RGR_UPDATE:
            handler = new Protocol::Mngr::Handler::RgrUpdate(conn, ev);
            break;

          case Protocol::Common::DO_ECHO:
            handler = new Protocol::Common::Handler::Echo(conn, ev);
            break;

          default: 
            handler = new Protocol::Common::Handler::NotImplemented(conn, ev);
            break;
        }

        if(handler)
          asio::post(*Env::IoCtx::io()->ptr(), 
                    [hdlr=AppHandlerPtr(handler)](){ hdlr->run();  });

        break;
      }

      default:
        HT_WARNF("Unimplemented event-type (%llu)", (Llu)ev->type);
        break;

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig==0){ // set signals listener
      Env::IoCtx::io()->signals()->async_wait(
        [ptr=this](const std::error_code &ec, const int &sig){
          HT_INFOF("Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          ptr->shutting_down(ec, sig); 
        }
      ); 
      HT_INFOF("Listening for Shutdown signal, set at sig=%d ec=%s", 
              sig, ec.message().c_str());
      return;
    }

    HT_INFOF("Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());
    
    (new std::thread([ptr=shared_from_this()]{ ptr->stop(); }))->detach();
  }

  void stop() override {
    
    m_srv->stop_accepting(); // no further requests accepted
    
    Env::Rangers::get()->stop();
    Env::MngrRole::get()->stop();

    Env::Clients::get()->rgr_service->stop();
    Env::Clients::get()->mngr_service->stop();

    Env::IoCtx::io()->stop();
    Env::FsInterface::interface()->stop();
    
    m_srv->shutdown();
    
    HT_INFO("Exit");
    std::quick_exit(0);
  }

  private:
  SerializedServerPtr m_srv = nullptr;
  //ColmNameToIDMap columns;       // column-name > CID


};

}}}

#endif // swc_app_manager_AppContext_h
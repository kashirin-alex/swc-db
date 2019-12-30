/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_AppContext_h
#define swc_app_manager_AppContext_h

#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"
#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/core/comm/DispatchHandler.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/client/Clients.h"

#include "swcdb/db/Columns/Schemas.h"
#include "swcdb/db/Columns/Mngr/Columns.h"

#include "swcdb/manager/MngrRole.h"
#include "swcdb/manager/Rangers.h"

#include "swcdb/db/Protocol/Common/handlers/NotImplemented.h"
#include "swcdb/manager/handlers/MngrState.h"
#include "swcdb/manager/handlers/MngrActive.h"
#include "swcdb/manager/handlers/ColumnMng.h"
#include "swcdb/manager/handlers/ColumnUpdate.h"
#include "swcdb/manager/handlers/ColumnGet.h"
#include "swcdb/manager/handlers/ColumnList.h"
#include "swcdb/manager/handlers/RgrMngId.h"
#include "swcdb/manager/handlers/RgrUpdate.h"
#include "swcdb/manager/handlers/RgrGet.h"
#include "swcdb/manager/handlers/Echo.h"


namespace SWC { namespace server { namespace Mngr {


class AppContext : public SWC::AppContext {
   
  // in-order of Protocol::Mngr::Command
  static constexpr const AppHandler_t handlers[] = { 
    &Protocol::Common::Handler::not_implemented,
    &Protocol::Mngr::Handler::mngr_state,
    &Protocol::Mngr::Handler::mngr_active,
    &Protocol::Mngr::Handler::column_mng,
    &Protocol::Mngr::Handler::column_update,
    &Protocol::Mngr::Handler::column_get,
    &Protocol::Mngr::Handler::column_list,
    &Protocol::Mngr::Handler::rgr_mng_id,
    &Protocol::Mngr::Handler::rgr_update,
    &Protocol::Mngr::Handler::rgr_get,
    &Protocol::Mngr::Handler::do_echo,
    //&Handler::debug,
    //&Handler::status,
    //&Handler::shutdown
  };

  public:

  AppContext() {
    Env::Config::settings()->parse_file(
      Env::Config::settings()->get<std::string>("swc.mngr.cfg", ""),
      Env::Config::settings()->get<std::string>("swc.mngr.OnFileChange.cfg", "")
    );

    Env::IoCtx::init(
      Env::Config::settings()->get<int32_t>("swc.mngr.handlers"));

    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get<std::string>("swc.fs")));
      
    Env::MngrRole::init();
    
    Env::Schemas::init();
    Env::MngrColumns::init();
    Env::Clients::init(
      std::make_shared<client::Clients>(
        Env::IoCtx::io()->shared(),
        std::make_shared<client::Mngr::AppContext>()
      )
    );
    Env::Rangers::init();
  }
  
  void init(const EndPoints& endpoints) override {
    Env::MngrRole::get()->init(endpoints);
    
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);
  }

  void set_srv(SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext(){}


  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Event::Type::ESTABLISHED:
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
        uint8_t cmd = ev->header.command >= Protocol::Mngr::MAX_CMD
                      ? Protocol::Mngr::NOT_IMPLEMENTED : ev->header.command;
        asio::post(
          *Env::IoCtx::io()->ptr(), 
          [cmd, conn, ev]() { 
            handlers[cmd](conn, ev); 
          }
        );
        return;
      }

      default:
        SWC_LOGF(LOG_WARN, "Unimplemented event-type (%llu)", (Llu)ev->type);
        break;

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig==0){ // set signals listener
      Env::IoCtx::io()->signals()->async_wait(
        [ptr=this](const std::error_code &ec, const int &sig){
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          ptr->shutting_down(ec, sig); 
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
    
    Env::Rangers::get()->stop();
    Env::MngrRole::get()->stop();

    Env::Clients::get()->rgr_service->stop();
    Env::Clients::get()->mngr_service->stop();

    Env::IoCtx::io()->stop();
    Env::FsInterface::interface()->stop();
    
    m_srv->shutdown();
    
    SWC_LOG(LOG_INFO, "Exit");
    std::quick_exit(0);
  }

  private:
  SerializedServer::Ptr m_srv = nullptr;
  //ColmNameToIDMap columns;       // column-name > CID


};

}}}

#endif // swc_app_manager_AppContext_h
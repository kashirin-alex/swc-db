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

#include "swcdb/lib/db/Columns/Schema.h"

#include "RoleState.h"
#include "RangeServers.h"

#include "handlers/MngrsState.h"
#include "handlers/ActiveMngr.h"
#include "handlers/MngRsId.h"
#include "handlers/UpdateRangeServers.h"


namespace SWC { namespace server { namespace Mngr {


class AppContext : public SWC::AppContext {
  
  public:

  AppContext() {
    EnvConfig::settings()->parse_file(
      EnvConfig::settings()->get<String>("swc.mngr.cfg", ""),
      EnvConfig::settings()->get<String>("swc.mngr.OnFileChange.cfg", "")
    );

    EnvIoCtx::init(EnvConfig::settings()->get<int32_t>("swc.mngr.handlers"));
    EnvMngrRoleState::init();
    EnvFsInterface::init();
    EnvSchemas::init();
    EnvMngrColumns::init();
    EnvClients::init(std::make_shared<client::Clients>(
      EnvIoCtx::io()->shared(),
      std::make_shared<client::Mngr::AppContext>()
    ));
    EnvRangeServers::init();
  }
  
  void init(EndPoints endpoints) override {
    EnvMngrRoleState::get()->init(endpoints);
    
    int sig = 0;
    EnvIoCtx::io()->set_signals();
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
        if(EnvMngrRoleState::get()->disconnection(
                          conn->endpoint_remote, conn->endpoint_local, true))
          return;
        return;

      case Event::Type::ERROR:
        //rangeservers->decommision(event->addr);
        break;

      case Event::Type::MESSAGE: {
        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Command::MNGR_REQ_MNGRS_STATE:
            handler = new Handler::MngrsState(conn, ev);
            break;

          case Protocol::Command::MNGR_UPDATE_RANGESERVERS:
            handler = new Handler::UpdateRangeServers(conn, ev);
            break;

          case Protocol::Command::CLIENT_REQ_ACTIVE_MNGR:
            handler = new Handler::ActiveMngr(conn, ev);
            break;

          case Protocol::Command::REQ_MNGR_MNG_RS_ID:
            handler = new Handler::MngRsId(conn, ev);
            break;
/* 
          case Protocol::Command::CLIENT_REQ_MNG_COLUMN:
            handler = new Handler::MngColumn(conn, ev);
            break;
*/
          // + MngrRsResource
          

          case Protocol::Command::CLIENT_REQ_RS_ADDR:
            //rangeservers->get_addr(event);
            break;

          case Protocol::Command::CLIENT_REQ_CID_NAME:
            //columns->get_cid_of_name(event);
            break;

          default: {           

            /*
            ev_ctx->error(Error::PROTOCOL_ERROR,
              format("Unimplemented command (%llu)", 
                    (Llu)ev_ctx->event->header.command));
            HT_ERRORF("Unimplemented protocol command (%llu)",
                      (Llu)ev_ctx->event->header.command);
                      */
          }
        }

        if(handler)
          asio::post(*EnvIoCtx::io()->ptr(), 
                    [hdlr=AppHandlerPtr(handler)](){ hdlr->run();  });

        break;
      }

      default:
        HT_THROWF(Error::PROTOCOL_ERROR, "Unimplemented event-type (%llu)",
                  (Llu)ev->type);

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig==0){ // set signals listener
      EnvIoCtx::io()->signals()->async_wait(
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
    
    EnvRangeServers::get()->stop();
    EnvMngrRoleState::get()->stop();

    EnvClients::get()->rs_service->stop();
    EnvClients::get()->mngr_service->stop();

    EnvFsInterface::interface()->stop();
    
    EnvIoCtx::io()->stop();
    
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
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_rangeserver_AppContext_h
#define swc_app_rangeserver_AppContext_h
#include <memory>

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"
#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "swcdb/lib/client/Clients.h"
#include "AppContextClient.h"

#include "swcdb/lib/db/Protocol/Commands.h"
#include "swcdb/lib/db/Protocol/req/ActiveMngr.h"

#include "swcdb/lib/db/Columns/Columns.h"

#include "callbacks/HandleRsAssign.h"
#include "callbacks/HandleRsShutdown.h"

#include "handlers/IsRangeLoaded.h"
#include "handlers/LoadRange.h"
#include "handlers/UnloadRange.h"


namespace SWC { namespace server { namespace RS {



class AppContext : public SWC::AppContext {
  public:

  AppContext() {
    
    EnvIoCtx::init(EnvConfig::settings()->get<int32_t>("swc.rs.handlers"));
    EnvFsInterface::init();
    EnvRsData::init();
    EnvColumns::init();

  }

  void init(EndPoints endpoints) override {
    EnvRsData::get()->endpoints = endpoints;
    
    int sig = 0;
    EnvIoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    EnvClients::init(std::make_shared<client::Clients>(
      EnvIoCtx::io()->shared(),
      std::make_shared<client::RS::AppContext>()
    ));
  
    mngr_root = std::make_shared<Protocol::Req::ActiveMngr>(1, 1);
    Protocol::Rsp::ActiveMngrRspCbPtr cb_hdlr 
      = std::make_shared<HandleRsAssign>(mngr_root);
    mngr_root->set_cb(cb_hdlr);
    mngr_root->run();
  }

  void set_srv(SerializedServerPtr srv){
    m_srv = srv;
  }

  virtual ~AppContext(){}


  void handle(ConnHandlerPtr conn, EventPtr ev) override {

    HT_DEBUGF("handle: %s", ev->to_str().c_str());
    
    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED:
        return;
        
      case Event::Type::DISCONNECT:
        return;

      case Event::Type::ERROR:
        return;

      case Event::Type::MESSAGE: {
        
        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Command::REQ_RS_IS_RANGE_LOADED: 
            handler = new Handler::IsRangeLoaded(conn, ev);
            break;
          case Protocol::Command::REQ_RS_LOAD_RANGE: 
            handler = new Handler::LoadRange(conn, ev);
            break;
          case Protocol::Command::REQ_RS_UNLOAD_RANGE: 
            handler = new Handler::UnloadRange(conn, ev);
            break;
            
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

        if(handler) //handler->run();
          asio::post(*EnvIoCtx::io()->ptr(), [handler](){ handler->run();  });

        break;
      }

      default:
        HT_WARNF("Unhandled, %s", ev->to_str().c_str());

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

    EnvColumns::get()->unload_all();

    Protocol::Rsp::ActiveMngrRspCbPtr cb_hdlr = 
      std::make_shared<HandleRsShutdown>(mngr_root, [this](){stop();});
    mngr_root->set_cb(cb_hdlr);
    
    mngr_root->run();
  }

  void stop(){
    HT_INFO("Stopping APP-RS");
    
    m_srv->stop_accepting(); // no further requests accepted

    EnvIoCtx::io()->stop();
    EnvFsInterface::fs()->stop();

    m_srv->shutdown();
  }

  private:
  SerializedServerPtr m_srv = nullptr;

  Protocol::Req::ActiveMngrPtr  mngr_root;
  
};

}}}

#endif // swc_app_rangeserver_AppContext_h
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_rangeserver_AppContext_h
#define swc_app_rangeserver_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"
#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "swcdb/lib/client/Clients.h"
#include "AppContextClient.h"

#include "swcdb/lib/db/Protocol/Commands.h"
#include "swcdb/lib/db/Protocol/req/ActiveMngrBase.h"

#include "swcdb/lib/db/Columns/Schema.h"
#include "swcdb/lib/db/Columns/RS/Columns.h"

#include "swcdb/lib/db/Protocol/req/MngRsId.h"

#include "swcdb/lib/db/Protocol/handlers/NotImplemented.h"
#include "handlers/IsRangeLoaded.h"
#include "handlers/LoadRange.h"
#include "handlers/UpdateSchema.h"
#include "handlers/UnloadRange.h"
#include "handlers/ColumnDelete.h"


namespace SWC { namespace server { namespace RS {



class AppContext : public SWC::AppContext {
  public:

  AppContext() {
    Env::Config::settings()->parse_file(
      Env::Config::settings()->get<String>("swc.rs.cfg", ""),
      Env::Config::settings()->get<String>("swc.rs.OnFileChange.cfg", "")
    );

    Env::IoCtx::init(
      Env::Config::settings()->get<int32_t>("swc.rs.handlers"));
    Env::FsInterface::init();
    Env::RsData::init();
    Env::Schemas::init();
    Env::RsColumns::init();
  }

  void init(const EndPoints& endpoints) override {
    Env::RsData::get()->endpoints = endpoints;
    
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Clients::init(std::make_shared<client::Clients>(
      Env::IoCtx::io()->shared(),
      std::make_shared<client::RS::AppContext>()
    ));
    
    m_id_mngr = std::make_shared<Protocol::Req::MngRsId>();
    m_id_mngr->assign();

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
        return;

      case Event::Type::ERROR:
        return;

      case Event::Type::MESSAGE: {
        
        if(Env::RsData::get()->rs_id == 0 && 
          ev->header.command != Protocol::Command::REQ_RS_ASSIGN_ID_NEEDED){
          try{conn->send_error(Error::RS_NOT_READY, "", ev);}catch(...){}
          break;
        }

        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Command::REQ_RS_IS_RANGE_LOADED: 
            handler = new Handler::IsRangeLoaded(conn, ev);
            break;

          case Protocol::Command::REQ_RS_LOAD_RANGE: 
            handler = new Handler::LoadRange(conn, ev);
            break;

          case Protocol::Command::REQ_RS_SCHEMA_UPDATE: 
            handler = new Handler::UpdateSchema(conn, ev);
            break;

          case Protocol::Command::REQ_RS_UNLOAD_RANGE: 
            handler = new Handler::UnloadRange(conn, ev);
            break;

          case Protocol::Command::REQ_RS_ASSIGN_ID_NEEDED:
            try{conn->response_ok(ev);}catch(...){}
            m_id_mngr->assign();
            break;
            
          case Protocol::Command::REQ_RS_COLUMN_DELETE: 
            handler = new Handler::ColumnDelete(conn, ev);
            break;
            
          case Protocol::Command::CLIENT_REQ_RS_ADDR:
            //rangeservers->get_addr(event);
            break;

          case Protocol::Command::CLIENT_REQ_CID_NAME:
            //columns->get_cid_of_name(event);
            break;

          default: 
            handler = new common::Handler::NotImplemented(conn, ev);
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

    m_srv->stop_accepting(); // no further requests accepted

    int err = Error::OK;
    Env::RsColumns::get()->unload_all(err, true);

    m_id_mngr->shutting_down(
      [ptr=shared_from_this()](){
        (new std::thread([ptr]{ ptr->stop(); }))->detach();
      }
    );
  }

  void stop() override {

    Env::Clients::get()->rs_service->stop();
    Env::Clients::get()->mngr_service->stop();
    
    Env::FsInterface::interface()->stop();
    
    Env::IoCtx::io()->stop();
    
    m_srv->shutdown();

    HT_INFO("Exit");
    std::quick_exit(0);
  }


  private:

  SerializedServerPtr           m_srv = nullptr;
  Protocol::Req::MngRsId::Ptr   m_id_mngr = nullptr;
  
};

}}}

#endif // swc_app_rangeserver_AppContext_h
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_ranger_AppContext_h
#define swc_app_ranger_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"
#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "swcdb/lib/client/Clients.h"
#include "AppContextClient.h"

#include "swcdb/lib/db/Protocol/Commands.h"

#include "swcdb/lib/db/Columns/Schema.h"
#include "swcdb/lib/db/Columns/Rgr/Columns.h"
#include "swcdb/lib/db/Columns/Rgr/Compaction.h"

#include "swcdb/lib/db/Protocol/Mngr/req/RgrMngId.h"

#include "swcdb/lib/db/Protocol/Common/handlers/NotImplemented.h"
#include "swcdb/lib/db/Protocol/Common/handlers/Echo.h"
#include "handlers/AssignId.h"
#include "handlers/RangeLoad.h"
#include "handlers/RangeUnload.h"
#include "handlers/RangeIsLoaded.h"
#include "handlers/RangeLocate.h"
#include "handlers/RangeQueryUpdate.h"
#include "handlers/RangeQuerySelect.h"
#include "handlers/ColumnUpdate.h"
#include "handlers/ColumnDelete.h"


namespace SWC { namespace server { namespace Rgr {



class AppContext : public SWC::AppContext {
  public:

  AppContext() {
    Env::Config::settings()->parse_file(
      Env::Config::settings()->get<String>("swc.rgr.cfg", ""),
      Env::Config::settings()->get<String>("swc.rgr.OnFileChange.cfg", "")
    );

    Env::IoCtx::init(
      Env::Config::settings()->get<int32_t>("swc.rgr.handlers"));
    Env::FsInterface::init();
    Env::RgrData::init();
    Env::Schemas::init();
    Env::RgrColumns::init();
  }

  void init(const EndPoints& endpoints) override {
    Env::RgrData::get()->endpoints = endpoints;
    
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Clients::init(std::make_shared<client::Clients>(
      Env::IoCtx::io()->shared(),
      std::make_shared<client::Rgr::AppContext>()
    ));

    m_id_validator 
      = std::make_shared<Protocol::Mngr::Req::RgrMngId::Scheduler>();
    Protocol::Mngr::Req::RgrMngId::assign(m_id_validator);

    m_compaction = std::make_shared<Compaction>();
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
        
        if(Env::RgrData::get()->id == 0 && 
          ev->header.command != Protocol::Rgr::ASSIGN_ID_NEEDED){
          try{conn->send_error(Error::RS_NOT_READY, "", ev);}catch(...){}
          break;
        }

        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Rgr::ASSIGN_ID_NEEDED:
            handler = new Protocol::Rgr::Handler::AssignId(
              conn, ev, m_id_validator);
            break;

          case Protocol::Rgr::COLUMN_DELETE: 
            handler = new Protocol::Rgr::Handler::ColumnDelete(conn, ev);
            break;

          case Protocol::Rgr::SCHEMA_UPDATE: 
            handler = new Protocol::Rgr::Handler::ColumnUpdate(conn, ev);
            break;

          case Protocol::Rgr::RANGE_IS_LOADED: 
            handler = new Protocol::Rgr::Handler::RangeIsLoaded(conn, ev);
            break;

          case Protocol::Rgr::RANGE_LOAD: 
            handler = new Protocol::Rgr::Handler::RangeLoad(conn, ev);
            break;

          case Protocol::Rgr::RANGE_UNLOAD: 
            handler = new Protocol::Rgr::Handler::RangeUnload(conn, ev);
            break;

          case Protocol::Rgr::RANGE_LOCATE: 
            handler = new Protocol::Rgr::Handler::RangeLocate(conn, ev);
            break;

          case Protocol::Rgr::RANGE_QUERY_UPDATE: 
            handler = new Protocol::Rgr::Handler::RangeQueryUpdate(conn, ev);
            break;

          case Protocol::Rgr::RANGE_QUERY_SELECT: 
            handler = new Protocol::Rgr::Handler::RangeQuerySelect(conn, ev);
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
        //std::cout << " cmd=" << ev->header.command << "\n";
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
    Env::RgrData::shuttingdown();
    HT_INFOF("Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());

    m_srv->stop_accepting(); // no further requests accepted

    Env::RgrColumns::get()->unload_all(false);

    Protocol::Mngr::Req::RgrMngId::shutting_down(
      m_id_validator,
      [ptr=shared_from_this()](){
        (new std::thread([ptr]{ ptr->stop(); }))->detach();
      }
    );
  }

  void stop() override {
    while(Env::RgrData::in_process() > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Env::RgrColumns::get()->unload_all(true); //re-check

    m_id_validator->stop();
    
    Env::Clients::get()->rgr_service->stop();
    Env::Clients::get()->mngr_service->stop();
    
    Env::IoCtx::io()->stop();
    Env::FsInterface::interface()->stop();
    
    m_srv->shutdown();

    HT_INFO("Exit");
    std::quick_exit(0);
  }


  private:
  
  std::mutex                m_mutex;
  SerializedServerPtr       m_srv = nullptr;
  Compaction::Ptr           m_compaction;
  
  Protocol::Mngr::Req::RgrMngId::Scheduler::Ptr   m_id_validator;
  
};

}}}

#endif // swc_app_ranger_AppContext_h
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

#include "callbacks/HandleRsAssign.h"
#include "callbacks/HandleRsShutdown.h"

#include "columns/Columns.h"

#include "handlers/LoadRange.h"


namespace SWC { namespace server { namespace RS {


class AppContext : public SWC::AppContext {
  public:

  AppContext() 
    : m_ioctx(std::make_shared<asio::io_context>(
      Config::settings->get<int32_t>("swc.rs.handlers"))),
      m_wrk(asio::make_work_guard(*m_ioctx.get())),
      m_columns(std::make_shared<Columns>())
  {

    (new std::thread(
      [this]{ 
        do{
          m_ioctx->run();
          std::cout << "app-ctx IO stopped, restarting\n";
          m_ioctx->restart();
        }while(m_run.load());
        std::cout << "app-ctx IO exited\n";
      }))->detach();
  }
  void init(EndPoints endpoints) override {
    
    int sig = 0;
    m_signals = std::make_shared<asio::signal_set>(*m_ioctx.get(), SIGINT, SIGTERM);
    shutting_down(std::error_code(), sig);

    m_endpoints = endpoints;

    m_clients = std::make_shared<client::Clients>(
      m_ioctx,
      std::make_shared<client::RS::AppContext>()
    );
    
    mngr_root = std::make_shared<Protocol::Req::ActiveMngr>(
      m_clients, 1, 3);
    Protocol::Rsp::ActiveMngrRspCbPtr cb_hdlr = 
      std::make_shared<HandleRsAssign>(m_endpoints, m_clients, mngr_root, rs_id);
    mngr_root->set_cb(cb_hdlr);
    mngr_root->run();
  }
  std::shared_ptr<asio::signal_set> m_signals;

  virtual ~AppContext(){}


  void handle(ConnHandlerPtr conn, EventPtr ev) override {

    std::cout << "handle:" << ev->to_str() << "\n";
    
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

          case Protocol::Command::MNGR_REQ_LOAD_RANGE: 
            handler = new Handler::LoadRange(conn, ev, m_columns);
            //(rid-barrier-release)
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
          asio::post(*m_ioctx.get(), [handler](){ handler->run();  });

        break;
      }

      default:
        HT_WARNF("Unhandled, %s", ev->to_str().c_str());

    }
    
  }

  void set_srv(SerializedServerPtr srv){
    m_srv = srv;
  }

  void shutting_down(const std::error_code &ec, const int &sig) {
    
    if(sig==0){ // set signals listener
      m_signals->async_wait(
        [ptr=this](const std::error_code &ec, const int &sig){
          ptr->shutting_down(ec, sig); 
        }
      ); 
      return;
    }

    HT_INFOF("Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());

    Protocol::Rsp::ActiveMngrRspCbPtr cb_hdlr = 
      std::make_shared<HandleRsShutdown>(
        m_endpoints, m_clients, mngr_root, rs_id, [this](){stop();});
    mngr_root->set_cb(cb_hdlr);
    
    mngr_root->run();
  }

  void stop(){
    m_srv->stop(); // no further requests accepted
    // fs(commit)
    
    m_clients->stop();

    m_run.store(false);
    m_wrk.get_executor().context().stop();
  }

  private:

  SerializedServerPtr m_srv = nullptr;
  std::atomic<bool>   m_run = true;
  IOCtxPtr            m_ioctx;
  asio::executor_work_guard<asio::io_context::executor_type> m_wrk; 

  ColumnsPtr                    m_columns;
  client::ClientsPtr            m_clients;
  Protocol::Req::ActiveMngrPtr  mngr_root;
  
  uint64_t rs_id = 0;
};

}}}

#endif // swc_app_rangeserver_AppContext_h
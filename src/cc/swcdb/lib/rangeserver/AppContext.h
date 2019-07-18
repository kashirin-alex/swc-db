/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_rangeserver_AppContext_h
#define swc_app_rangeserver_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/client/Clients.h"
#include "AppContextMngrClient.h"

#include "swcdb/lib/db/Protocol/Commands.h"
#include "swcdb/lib/db/Protocol/req/ActiveMngr.h"

#include "callbacks/HandleRsAssign.h"

#include "columns/Columns.h"

#include <memory>


namespace SWC { namespace server { namespace RS {


class AppContext : public SWC::AppContext {
  public:

  AppContext() 
    : m_ioctx(std::make_shared<asio::io_context>(
      Config::settings->get<int32_t>("swc.rs.handlers"))),
      m_wrk(asio::make_work_guard(*m_ioctx.get()))
  {
    (new std::thread(
      [this]{ 
        do{
          m_ioctx->run();
          std::cout << "IO stopped, restarting\n";
          m_ioctx->restart();
        }while(m_run.load());
        std::cout << "IO exited\n";
      }))->detach();
  }

  void init(EndPoints endpoints) override {
    m_endpoints = endpoints;

    m_clients = std::make_shared<client::Clients>(
      m_ioctx,
      std::make_shared<client::Mngr::AppContext>()
    );
    

    mngr_root = std::make_shared<Protocol::Req::ActiveMngr>(
      m_clients, 1, 3);
    Protocol::Rsp::ActiveMngrRspCbPtr cb_hdlr = 
      std::make_shared<HandleRsAssign>(m_endpoints, m_clients, mngr_root, rs_id);
    mngr_root->set_cb(cb_hdlr);
    
    mngr_root->run();
  }

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

          case Protocol::Command::RS_RSP_RANGE_LOADED:
            //(rid-barrier-release)
            //rangeservers->load_ranges(event->addr);
            break;

          case Protocol::Command::RS_REQ_SHUTTING_DOWN:
            //rangeservers->decommision(event->addr);
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

  private:
  std::atomic<bool> m_run = true;
  IOCtxPtr          m_ioctx;
  asio::executor_work_guard<asio::io_context::executor_type> m_wrk; 

  ColumnsPtr columns ();
  client::ClientsPtr m_clients;
  Protocol::Req::ActiveMngrPtr mngr_root;
  
  uint64_t rs_id = 0;
};

}}}

#endif // swc_app_rangeserver_AppContext_h
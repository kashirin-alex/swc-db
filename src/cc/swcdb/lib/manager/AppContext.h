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

#include "RoleState.h"

#include "swcdb/lib/manager/handlers/MngrsState.h"
#include "swcdb/lib/manager/handlers/ActiveMngr.h"
#include "swcdb/lib/manager/handlers/AssignRsId.h"

#include "swcdb/lib/manager/RangeServers.h"


namespace SWC { namespace server { namespace Mngr {

class AppContext : public SWC::AppContext {
  
  public:

  AppContext() :
    m_ioctx(std::make_shared<asio::io_context>(
      Config::settings->get<int32_t>("swc.mngr.handlers"))),
    m_wrk(std::make_shared<IO_DoWork>(asio::make_work_guard(*m_ioctx.get()))),
    m_fs(std::make_shared<FS::Interface>()),
    m_role_state(std::make_shared<RoleState>(m_ioctx)),
    m_rangeservers(std::make_shared<Mngr::RangeServers>(m_ioctx))
  {
    (new std::thread(
      [io_ptr=m_ioctx, run=&m_run]{ 
        do{
          io_ptr->run();
          HT_DEBUG("IO stopped, restarting");
          io_ptr->restart();
        }while(run->load());
        HT_DEBUG("IO exited");
      }))->detach();
  }
  
  void init(EndPoints endpoints) override {
    m_endpoints = endpoints;
    
    m_clients = std::make_shared<client::Clients>(
      m_ioctx, 
      std::make_shared<client::Mngr::AppContext>(m_role_state)
    );
    m_role_state->init(m_endpoints, m_clients);
    m_rangeservers->init(m_clients);
  }

  virtual ~AppContext(){}


  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    HT_DEBUGF("handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED:
        return;
        //rangeservers->decommision(event->addr); 
        break;
      case Event::Type::DISCONNECT:
        if(m_role_state->disconnection(
                          conn->endpoint_remote, conn->endpoint_local, true))
          return;
        //m_rangeservers->decommision(event->addr); 
        break;

      case Event::Type::ERROR:
        //rangeservers->decommision(event->addr);
        break;

      case Event::Type::MESSAGE: {
        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Command::MNGR_REQ_MNGRS_STATE:
            handler = new Handler::MngrsState(conn, ev, m_role_state);
            break;

          case Protocol::Command::CLIENT_REQ_ACTIVE_MNGR:
            handler = new Handler::ActiveMngr(conn, ev, m_role_state);
            break;

          case Protocol::Command::RS_REQ_MNG_RS_ID:
            handler = new Handler::AssignRsId(conn, ev, m_rangeservers, m_role_state);
            break;

          case Protocol::Command::RS_RSP_RANGE_LOADED:
            //(rid-barrier-release)
            //rangeservers->load_ranges(event->addr);
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
            HT_THROWF(Error::PROTOCOL_ERROR, "Unimplemented event-type (%llu)",
                      (Llu)ev->type);

    }

    //if(conn->is_open())
    //  conn->read_header();



    
  }

  void set_srv(SerializedServerPtr srv){
    m_srv = srv;
  }

  private:
  SerializedServerPtr m_srv = nullptr;
  std::atomic<bool>   m_run = true;
  IOCtxPtr            m_ioctx;
  IO_DoWorkPtr        m_wrk = nullptr;

  FS::InterfacePtr    m_fs;
  RoleStatePtr        m_role_state;
  RangeServersPtr     m_rangeservers;
  client::ClientsPtr  m_clients;
  //ColmNameToIDMap columns;       // column-name > CID

  /* 
  m_wrk->reset();
  //m_wrk->get_executor().context().stop();
  */

};

}}}

#endif // swc_app_manager_AppContext_h
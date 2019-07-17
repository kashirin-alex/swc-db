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

#include "RoleState.h"

#include "swcdb/lib/manager/handlers/AssignRsId.h"
#include "swcdb/lib/manager/handlers/IsMngrActive.h"
#include "swcdb/lib/manager/handlers/MngrsState.h"

#include "swcdb/lib/manager/MngrRangeServers.h"


namespace SWC { namespace server { namespace Mngr {

class AppContext : public SWC::AppContext {
  
  public:

  AppContext() :
    m_ioctx(std::make_shared<asio::io_context>(
      Config::settings->get<int32_t>("swc.mngr.handlers", 8))),
    m_wrk(std::make_shared<IO_DoWork>(asio::make_work_guard(*m_ioctx.get()))),
    m_role_state(std::make_shared<RoleState>(m_ioctx)),
    m_mngr_rs(std::make_shared<Mngr::MngrRangeServers>())
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
    m_role_state->init(m_endpoints);
  }

  virtual ~AppContext(){}


  std::atomic<uint32_t> num  = 0;

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
        //m_mngr_rs->decommision(event->addr); 
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

          case Protocol::Command::MNGR_REQ_IS_MNGR_ACTIVE:
            handler = new Handler::IsMngrActive(conn, ev, m_role_state);
            break;

          case Protocol::Command::RS_REQ_ASSIGN_RS_ID:
            /*
            if(!m_role_state->is_active(2))
              Protocol::Command::MNGR_RSP_CID_NOT_MANAGED
            */

            handler = new Handler::AssignRsId(conn, ev, m_mngr_rs);
            //rangeservers->add(event->addr);  // add addr and assign RS-N 
            break;

          case Protocol::Command::RS_RSP_ASSIGN_RS_ID_ACK:
            //rangeservers->load_ranges(event->addr); 
            // load_ranges(rid-barrier), if exists, else OK
            break;

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
            HT_THROWF(Error::PROTOCOL_ERROR, "Unimplemented event-type (%llu)",
                      (Llu)ev->type);

    }

    //if(conn->is_open())
    //  conn->read_header();



    
  }

  private:
  
  std::atomic<bool> m_run = true;
  IOCtxPtr          m_ioctx;
  IO_DoWorkPtr      m_wrk = nullptr;

  RoleStatePtr      m_role_state;
  /* 
  m_wrk->reset();
  //m_wrk->get_executor().context().stop();
  */

  MngrRangeServersPtr         m_mngr_rs;  // RS-N > addr/s
  //ColmNameToIDMap columns;       // column-name > CID
};

}}}

#endif // swc_app_manager_AppContext_h
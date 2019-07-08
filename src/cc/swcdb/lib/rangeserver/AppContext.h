/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_rangeserver_AppContext_h
#define swc_app_rangeserver_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/db/Protocol/Commands.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/mngr/AppContext.h"

#include "columns/Columns.h"

#include <memory>


namespace SWC { namespace server { namespace RS {


class AppContext : public SWC::AppContext {
  public:

  AppContext() 
    : m_ioctx(std::make_shared<asio::io_context>(
      Config::settings->get<int32_t>("swc.rs.handlers", 8))),
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
  
    m_clients = std::make_shared<client::Clients>(
      m_ioctx,
      std::make_shared<client::Mngr::AppContext>()
    );
    
    SWC::EndPoints endpoints = m_clients->mngrs_groups->get_endpoints(0, 2);
    
    client::ClientConPtr con_h = m_clients->mngr_service->get_connection(endpoints);
    con_h->accept_requests();
  }

  virtual ~AppContext(){}

  std::atomic<uint32_t> num  = 0;

  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    //if(ev->type != Event::Type::DISCONNECT){
    std::cout << "AppContext-RS, handle:" << ev->to_str() << "\n";

    //}
    return;
    // HT_INFOF("Received event-type (%llu)", (Llu)ev->type);

    
    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED:
        return;
        //rangeservers->decommision(event->addr);  // remove RS-N of addr & load-ranges to other
        break;
      case Event::Type::DISCONNECT:
        return;
        //m_mngr_rs->decommision(event->addr);  // remove RS-N of addr & load-ranges to other
        break;

      case Event::Type::ERROR:
      /* 
        ev->error(Error::PROTOCOL_ERROR,
          format("Received Error(%s)", ev_ctx->get_peer_addr().to_string()));
          */
        //rangeservers->decommision(event->addr);  // remove RS-N of addr & load-ranges to other
        break;

      case Event::Type::MESSAGE: {
        CommHeader header;
        header.initialize_from_request_header(ev->header);
        num++;      
        std::string s(format(" with=(%d)", num.load()));
        SWC::CommBufPtr cbp = std::make_shared<SWC::CommBuf>(header, s.length()+ev->payload_len);
      
        cbp->append_bytes(ev->payload, ev->payload_len);
        cbp->append_bytes((const uint8_t*)s.c_str(), s.length());
        conn->send_response(cbp);

        // conn->response_ok(ev);
        // ev_ctx->send_message("AppContext-Mngr", 14);
        /* 
        ev_ctx->error(Error::PROTOCOL_ERROR,
          format("Unimplemented command (%llu)", 
            (Llu)ev_ctx->event->header.command));
        */
        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Command::RS_REQ_ASSIGN_RS_ID:

            //handler = new Handler::AssignRsId(conn, ev, m_mngr_rs);
            //rangeservers->add(event->addr);  // add addr and assign RS-N 
            break;

          case Protocol::Command::RS_RSP_ASSIGN_RS_ID_ACK:
            //rangeservers->load_ranges(event->addr); // load_ranges(rid-barrier), if exists, else OK
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

        if(handler)
          handler->run();
        // asio::post(*m_io_ctx.get(), [handler](){ handler->run();  });

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
  asio::executor_work_guard<asio::io_context::executor_type> m_wrk; 

  ColumnsPtr columns ();
  client::ClientsPtr m_clients;
};

}}}

#endif // swc_app_rangeserver_AppContext_h
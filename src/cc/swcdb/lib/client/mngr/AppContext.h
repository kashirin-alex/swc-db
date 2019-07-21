/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_mngr_AppContext_h
#define swc_lib_client_mngr_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/db/Protocol/Commands.h"


namespace SWC { namespace client { namespace Mngr {


class AppContext : public SWC::AppContext {
  public:

  AppContext(){}

  virtual ~AppContext(){}


  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    //if(ev->type != Event::Type::DISCONNECT){
    std::cout << "AppContext-MngrClient, handle: " << ev->to_str() << "\n";
    //}
    return;

    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED:
        conn->send_error(SWC::Error::PROTOCOL_ERROR, 
                        format("Client send event, addr %s", 
                                conn->endpoint_remote_str().c_str()));
        //rangeservers->decommision(event->addr);  // remove RS-N of addr & load-ranges to other
        break;
      case Event::Type::DISCONNECT:
        //rangeservers->decommision(event->addr);  // remove RS-N of addr & load-ranges to other
        break;

      case Event::Type::ERROR:
        conn->send_error(Error::PROTOCOL_ERROR,
          format("Received Error(%s)", conn->endpoint_remote_str().c_str()));
        //rangeservers->decommision(event->addr);  // remove RS-N of addr & load-ranges to other
        break;

      case Event::Type::MESSAGE: {
      
        // conn->send_message("AppContext-MngrClient", 14);

        conn->send_error(Error::PROTOCOL_ERROR,
          format("Unimplemented command (%llu)", 
            (Llu)ev->header.command));

        AppHandler *handler = 0;
        switch (ev->header.command) {

          case Protocol::Command::RS_REQ_ASSIGN_RS_ID:

            //handler = new Handler::AssignRsId(ev, m_mngr_rs);
            //rangeservers->add(event->addr);  // add addr and assign RS-N 
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

            conn->send_error(Error::PROTOCOL_ERROR,
              format("Unimplemented command (%llu)", 
                    (Llu)ev->header.command));
            HT_ERRORF("Unimplemented protocol command (%llu)",
                      (Llu)ev->header.command);
          }
        }

        if(handler)
          handler->run();

        break;
      }

      default:
            HT_THROWF(Error::PROTOCOL_ERROR, "Unimplemented event-type (%llu)",
                      (Llu)ev->type);

    }



    
  }
  
};

}}}

#endif // swc_lib_client_mngr_AppContext_h
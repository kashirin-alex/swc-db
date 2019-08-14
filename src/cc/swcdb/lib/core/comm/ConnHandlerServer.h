/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandlerServer_h
#define swc_core_comm_ConnHandlerServer_h

#include "swcdb/lib/core/comm/ConnHandler.h"


namespace SWC { namespace server {


class ConnHandlerServer : public ConnHandler {

  public:
  ConnHandlerServer(AppContextPtr app_ctx, SocketPtr socket, IOCtxPtr io_ctx) 
                    : ConnHandler(app_ctx, socket, io_ctx){}

  virtual ~ConnHandlerServer(){}

  void new_connection() override {
    ConnHandler::new_connection();
    run(std::make_shared<Event>(Event::Type::CONNECTION_ESTABLISHED, Error::OK)); 
    accept_requests();
  }

  void run(EventPtr ev, DispatchHandlerPtr hdlr=nullptr) override {
    if(hdlr != nullptr)
      hdlr->handle(ptr(), ev);
    else if(m_app_ctx != nullptr) // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      m_app_ctx->handle(ptr(), ev); 
  }

};


}}

#endif // swc_core_comm_ConnHandlerServer_h
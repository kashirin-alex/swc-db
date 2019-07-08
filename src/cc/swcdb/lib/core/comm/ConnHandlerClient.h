/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandlerClient_h
#define swc_core_comm_ConnHandlerClient_h

#include "swcdb/lib/core/comm/ConnHandler.h"


namespace SWC { namespace client {


class ConnHandlerClient : public ConnHandler {

  public:
  ConnHandlerClient(AppContextPtr app_ctx, SocketPtr socket, IOCtxPtr io_ctx) 
                    : ConnHandler(app_ctx, socket, io_ctx){}

  virtual ~ConnHandlerClient() {
    do_close();
  }

  void new_connection() override {
    ConnHandler::new_connection();
    run(std::make_shared<Event>(Event::Type::CONNECTION_ESTABLISHED, Error::OK)); 
  }


  void run(EventPtr ev, DispatchHandlerPtr hdlr=nullptr) override {
    if(hdlr != nullptr)
      hdlr->handle(this, ev);
    else if(m_app_ctx != nullptr) // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      m_app_ctx->handle(this, ev); 
  }

};

typedef std::shared_ptr<ConnHandlerClient> ConnHandlerClientPtr;
}}

#endif // swc_core_comm_ConnHandlerClient_h
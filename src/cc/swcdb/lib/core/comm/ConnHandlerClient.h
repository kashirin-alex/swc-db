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

  virtual ~ConnHandlerClient(){}

  void new_connection() override {
    ConnHandler::new_connection();
    run(Event::make(Event::Type::ESTABLISHED, Error::OK)); 
  }


  void run(Event::Ptr ev, DispatchHandlerPtr hdlr=nullptr) override {
    if(hdlr != nullptr)
      hdlr->handle(ptr(), ev);
    else if(app_ctx != nullptr) // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      app_ctx->handle(ptr(), ev); 
  }

};

typedef std::shared_ptr<ConnHandlerClient> ConnHandlerClientPtr;
}}

#endif // swc_core_comm_ConnHandlerClient_h
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandlerClient_h
#define swc_core_comm_ConnHandlerClient_h

#include "ConnHandler.h"


namespace SWC { namespace client {


class ConnHandler : public SWC::ConnHandler {
  public:
  
  typedef std::shared_ptr<ConnHandler> Ptr;

  ConnHandler(AppContextPtr app_ctx, Socket& socket, IOCtxPtr io_ctx) 
              : SWC::ConnHandler(app_ctx, socket, io_ctx){
  }

  virtual ~ConnHandler(){}

  void new_connection() override {
    SWC::ConnHandler::new_connection();
    auto ev = Event::make(Event::Type::ESTABLISHED, Error::OK);
    run(ev); 
  }


  void run(Event::Ptr& ev) override {
    if(app_ctx != nullptr) // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      app_ctx->handle(ptr(), ev); 
  }

};

}}

#endif // swc_core_comm_ConnHandlerClient_h
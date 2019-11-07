/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_ConnHandlerServer_h
#define swc_core_comm_ConnHandlerServer_h

#include "swcdb/lib/core/comm/ConnHandler.h"


namespace SWC { namespace server {


class ConnHandler : public SWC::ConnHandler {

  public:

  ConnHandler(AppContextPtr app_ctx, Socket& socket, IOCtxPtr io_ctx) 
                    : SWC::ConnHandler(app_ctx, socket, io_ctx){
  }

  virtual ~ConnHandler(){}

  void new_connection() override {
    SWC::ConnHandler::new_connection();
    run(Event::make(Event::Type::ESTABLISHED, Error::OK)); 
    accept_requests();
  }
  
  void run(Event::Ptr ev, DispatchHandler::Ptr hdlr=nullptr) override {
    if(hdlr != nullptr)
      hdlr->handle(ptr(), ev);
    else if(app_ctx != nullptr) // && if(ev->header.flags & CommHeader::FLAGS_BIT_REQUEST)
      app_ctx->handle(ptr(), ev); 
  }

};


}}

#endif // swc_core_comm_ConnHandlerServer_h
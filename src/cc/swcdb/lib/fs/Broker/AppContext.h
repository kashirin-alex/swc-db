/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_AppContext_h
#define swc_lib_fs_Broker_AppContext_h

#include "swcdb/lib/client/AppContext.h"

namespace SWC{ namespace FS {

class FsClientAppCtx : public client::AppContext {
  public:
  FsClientAppCtx(){}
  virtual ~FsClientAppCtx(){}
    
  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    
    if(ev->type == Event::Type::DISCONNECT)
      return;
      
    const uint8_t *ptr;
    size_t remain;
    if(Protocol::Req::Base().is_rsp(conn, ev, ev->header.command, &ptr, &remain))
      HT_WARNF("Unhandled %s", ev->to_str().c_str());
  }
};

}}

#endif  // swc_lib_fs_Broker_AppContext_h
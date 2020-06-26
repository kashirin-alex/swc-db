/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_AppContext_h
#define swc_fs_Broker_AppContext_h

#include "swcdb/core/comm/AppContext.h"

namespace SWC{ namespace FS {

class AppContext final : public SWC::AppContext {
  public:

  AppContext(){}

  virtual ~AppContext(){}
    
  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {
    
    if(ev->type == Event::Type::DISCONNECT)
      return;
      
    const uint8_t *ptr;
    size_t remain;
    if(Protocol::Req::Base().is_rsp(conn, ev, ev->header.command, &ptr, &remain))
      SWC_LOGF(LOG_WARN, "Unhandled %s", ev->to_str().c_str());
  }
};

}}

#endif  // swc_fs_Broker_AppContext_h
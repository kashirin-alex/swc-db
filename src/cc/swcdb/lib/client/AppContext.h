/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_AppContext_h
#define swc_lib_client_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/db/Protocol/Commands.h"

#include <memory>

namespace SWC { namespace client { 


class AppContext : public SWC::AppContext {
  public:

  AppContext(){}

  virtual ~AppContext(){}

  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    //if(ev->type != Event::Type::DISCONNECT){
    std::cout << "ClientAppContext, handle: " << ev->to_str() << "\n";
    //}
    return;
  }
  
};

}}

#endif // swc_lib_client_AppContext_h
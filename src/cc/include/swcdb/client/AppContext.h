/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_AppContext_h
#define swc_lib_client_AppContext_h

#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace client { 

class AppContext : public SWC::AppContext {
  public:

  AppContext();

  virtual ~AppContext();

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;
  
};

}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/client/AppContext.cc"
#endif 

#endif // swc_lib_client_AppContext_h
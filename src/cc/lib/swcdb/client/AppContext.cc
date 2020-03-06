/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/client/AppContext.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace client { 

AppContext::AppContext() { }

AppContext::~AppContext() { }

void AppContext::handle(ConnHandlerPtr conn, Event::Ptr& ev) {
    //if(ev->type != Event::Type::DISCONNECT){
    // std::cout << "ClientAppContext, handle: " << ev->to_str() << "\n";
    //}
  return;
}

}}

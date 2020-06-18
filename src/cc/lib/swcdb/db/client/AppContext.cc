/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */
 
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace client { 

AppContext::AppContext() { }

AppContext::~AppContext() { }

void AppContext::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {
    //if(ev->type != Event::Type::DISCONNECT){
    // std::cout << "ClientAppContext, handle: " << ev->to_str() << "\n";
    //}
  return;
}

}}

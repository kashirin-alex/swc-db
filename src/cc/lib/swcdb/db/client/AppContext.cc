/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace client { 

AppContext::AppContext() { }

AppContext::~AppContext() { }

void AppContext::handle(ConnHandlerPtr, const Event::Ptr&) {
    /*
    if(ev->type != Event::Type::DISCONNECT) {
     SWC_PRINT << "ClientAppContext, handle: " << ev->to_str() 
        << SWC_PRINT_CLOSE;
    }
    */
}

}}

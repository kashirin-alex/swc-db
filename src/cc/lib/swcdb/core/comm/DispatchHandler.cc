/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Error.h"
#include "swcdb/core/comm/DispatchHandler.h"

namespace SWC {


void DispatchHandler::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  SWC_LOG_OUT(LOG_WARN,
    ev->print(SWC_LOG_OSTREAM << "DispatchHandler(handle is Virtual!)\n");
  );
}
    
bool DispatchHandler::run() { 
  return false; 
}


}

/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Error.h"
#include "swcdb/core/comm/DispatchHandler.h"

namespace SWC {

DispatchHandler::~DispatchHandler() { }

void DispatchHandler::handle(ConnHandlerPtr conn, const Event::Ptr& ev) { 
  SWC_LOGF(LOG_WARN, "handle(virtual): %s", ev->to_str().c_str());
  return;
}
    
bool DispatchHandler::run(uint32_t timeout) { 
  return false; 
}
  
}
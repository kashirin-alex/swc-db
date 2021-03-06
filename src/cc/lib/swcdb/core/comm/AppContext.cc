/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Logger.h"
#include "swcdb/core/comm/AppContext.h"


namespace SWC { namespace Comm {


void AppContext::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  SWC_LOG_OUT(LOG_WARN,
    ev->print(SWC_LOG_OSTREAM << "AppContext(handle is Virtual!)\n");
  );
}

void AppContext::stop() {
  SWC_LOG(LOG_WARN, "AppContext(stop is Virtual)!");
}


}}

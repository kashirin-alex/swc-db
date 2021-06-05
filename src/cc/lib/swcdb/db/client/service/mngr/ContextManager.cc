/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/client/service/mngr/ContextManager.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace client {

ContextManager::ContextManager(const Config::Settings& settings)
    : Comm::AppContext(
        settings.get<Config::Property::V_GENUM>(
          "swc.client.Mngr.comm.encoder")) {
}

void ContextManager::handle(Comm::ConnHandlerPtr, //conn
                            const Comm::Event::Ptr& ev) {
  SWC_LOG_OUT(LOG_WARN,  ev->print(SWC_LOG_OSTREAM << "unhandled: "); );
}


}}

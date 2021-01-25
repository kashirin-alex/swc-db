/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/client/Settings.h"
#include "swcdb/db/client/ContextManager.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace client {

ContextManager::ContextManager()
    : Comm::AppContext(
        Env::Config::settings()->get<Config::Property::V_GENUM>(
          "swc.client.Mngr.comm.encoder")) {
}

ContextManager::~ContextManager() { }

void ContextManager::handle(Comm::ConnHandlerPtr, //conn
                            const Comm::Event::Ptr& ev) {
  switch (ev->type) {

    case Comm::Event::Type::ERROR:
    case Comm::Event::Type::MESSAGE: {
      SWC_LOG_OUT(LOG_WARN,  ev->print(SWC_LOG_OSTREAM << "unhandled: "); );
      break;
    }

    case Comm::Event::Type::DISCONNECT:
    case Comm::Event::Type::ESTABLISHED:
    default: {
      break;
    }
  }
}


}}

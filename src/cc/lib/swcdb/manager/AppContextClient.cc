/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace client { namespace Mngr { 

AppContext::AppContext() { }

AppContext::~AppContext() { }

void AppContext::disconnected(const ConnHandlerPtr& conn) {
  Env::Mngr::role()->disconnection(
    conn->endpoint_remote, conn->endpoint_local);
}

void AppContext::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {

  switch (ev->type) {

    case Event::Type::DISCONNECT: {
      disconnected(conn);
      return;
    }

    case Event::Type::MESSAGE:
    case Event::Type::ERROR: {
      SWC_LOG_OUT(LOG_WARN,  ev->print(SWC_LOG_OSTREAM << "unhandled: "); );
      break;
    }

    case Event::Type::ESTABLISHED:
    default: {
      break;
    }

  }
    
}
  

}}}
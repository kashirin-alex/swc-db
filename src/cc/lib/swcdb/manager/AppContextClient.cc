/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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

    case Event::Type::ESTABLISHED: {
      break;
    }
      
    case Event::Type::DISCONNECT: {
      disconnected(conn);
      return;
    }

    case Event::Type::ERROR: {
      SWC_LOGF(LOG_WARN, "unhandled: %s", ev->to_str().c_str());
      break;
    }

    case Event::Type::MESSAGE: {
      SWC_LOGF(LOG_WARN, "unhandled: %s", ev->to_str().c_str());
      break;
    }

    default: {
      break;
    }

  }
    
}
  

}}}
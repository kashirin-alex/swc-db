/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_AppHandler_h
#define swc_core_comm_AppHandler_h

#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC {

namespace { typedef void handler(ConnHandlerPtr conn, Event::Ptr ev); }
typedef handler* AppHandler_t;


class AppHandler {
  public:

  AppHandler(ConnHandlerPtr conn, Event::Ptr ev): m_conn(conn), m_ev(ev) { }
    
  virtual ~AppHandler() { }

  virtual void run() = 0;

  protected:
  ConnHandlerPtr  m_conn;
  Event::Ptr      m_ev;
};

} // namespace SWC

#endif // swc_core_comm_AppHandler_h

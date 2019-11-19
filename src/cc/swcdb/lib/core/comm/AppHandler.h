/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_AppHandler_h
#define swc_core_comm_AppHandler_h

#include "Event.h"
#include "ConnHandler.h"

namespace SWC {

class AppHandler {

  public:
  typedef std::unique_ptr<AppHandler> Ptr;

  AppHandler(ConnHandlerPtr conn, Event::Ptr ev)
             : m_conn(conn), m_ev(ev) {
  }
    
  virtual ~AppHandler() { }

  virtual void run() = 0;

  protected:
  ConnHandlerPtr  m_conn;
  Event::Ptr      m_ev;
};

} // namespace SWC

#endif // swc_core_comm_AppHandler_h

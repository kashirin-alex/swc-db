/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_AppHandler_h
#define swcdb_core_comm_AppHandler_h

#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace Comm {

typedef void (*AppHandler_t)(const ConnHandlerPtr& conn,
                             const Event::Ptr& ev);

class AppHandler {
  public:

  SWC_CAN_INLINE
  AppHandler(const ConnHandlerPtr& conn, const Event::Ptr& ev) noexcept
            : m_conn(conn), m_ev(ev) {
  }

  virtual void run() = 0;

  protected:

  virtual ~AppHandler() { }

  ConnHandlerPtr  m_conn;
  Event::Ptr      m_ev;
};


}} // namespace SWC::Comm



#endif // swcdb_core_comm_AppHandler_h

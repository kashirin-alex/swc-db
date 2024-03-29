/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_DispatchHandler_h
#define swcdb_core_comm_DispatchHandler_h


#include "swcdb/core/comm/Event.h"


namespace SWC { namespace Comm {


class DispatchHandler : public std::enable_shared_from_this<DispatchHandler> {
  public:

  typedef std::shared_ptr<DispatchHandler> Ptr;

  virtual void handle_no_conn() = 0;

  virtual void handle(ConnHandlerPtr conn, const Event::Ptr& ev) = 0;

  virtual bool run() { return false; }

  virtual bool valid() { return true; }

  protected:

  virtual ~DispatchHandler() noexcept { }

};


}} // namespace SWC::Comm




#endif // swcdb_core_comm_DispatchHandler_h

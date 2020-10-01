/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_DispatchHandler_h
#define swcdb_core_comm_DispatchHandler_h


#include <memory>
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/AppContext.h"

namespace SWC { namespace Comm {


class DispatchHandler : public std::enable_shared_from_this<DispatchHandler> {
  public:

  typedef std::shared_ptr<DispatchHandler> Ptr;

  virtual void handle(ConnHandlerPtr conn, const Event::Ptr& ev);
    
  virtual bool run();
  
  virtual ~DispatchHandler() { };

};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/DispatchHandler.cc"
#endif 

#endif // swcdb_core_comm_DispatchHandler_h

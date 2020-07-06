/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_comm_DispatchHandler_h
#define swc_core_comm_DispatchHandler_h


#include <memory>
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/AppContext.h"

namespace SWC {


class DispatchHandler : public std::enable_shared_from_this<DispatchHandler> {
  public:

  typedef std::shared_ptr<DispatchHandler> Ptr;

  virtual void handle(ConnHandlerPtr conn, const Event::Ptr& ev);
    
  virtual bool run();
  
  ~DispatchHandler();
};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/DispatchHandler.cc"
#endif 

#endif // swc_core_comm_DispatchHandler_h

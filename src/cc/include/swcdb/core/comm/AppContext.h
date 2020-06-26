
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_comm_AppContext_h
#define swc_core_comm_AppContext_h

#include <memory>
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC {

// forward declarations
class ConnHandler;
typedef std::shared_ptr<ConnHandler> ConnHandlerPtr;


class AppContext : public std::enable_shared_from_this<AppContext> {
  public:
  
  typedef std::shared_ptr<AppContext> Ptr;

  AppContext();

  virtual ~AppContext();

  virtual void handle(ConnHandlerPtr conn, const Event::Ptr& ev);

  virtual void init(const EndPoints& endpoints);

  virtual void stop();

  EndPoints m_endpoints;
};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/AppContext.cc"
#endif 

#endif // swc_core_comm_AppContext_h
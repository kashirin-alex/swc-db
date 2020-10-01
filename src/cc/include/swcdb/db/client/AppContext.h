/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_AppContext_h
#define swcdb_db_client_AppContext_h

#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace client { 

class AppContext : public SWC::Comm::AppContext {
  public:

  AppContext();

  virtual ~AppContext();

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;
  
};

}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/AppContext.cc"
#endif 

#endif // swcdb_db_client_AppContext_h
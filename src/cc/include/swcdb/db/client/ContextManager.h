/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_ContextManager_h
#define swcdb_db_client_ContextManager_h

#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace client { 

class ContextManager : public Comm::AppContext {
  public:

  typedef std::shared_ptr<ContextManager> Ptr;

  ContextManager();

  virtual ~ContextManager();

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

};

}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/ContextManager.cc"
#endif 

#endif // swcdb_db_client_ContextManager_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_rgr_Rangers_h
#define swcdb_db_client_rgr_Rangers_h


#include "swcdb/db/client/service/rgr/ContextRanger.h"
#include "swcdb/db/client/service/rgr/Cache.h"
#include "swcdb/core/comm/ClientConnQueues.h"


namespace SWC { namespace client {



class Rangers {
  public:

  SWC_CAN_INLINE
  Rangers() noexcept : queues(nullptr), cache(nullptr) { }

  Rangers(const Config::Settings& settings,
          Comm::IoContextPtr ioctx,
          const ContextRanger::Ptr& rgr_ctx);

  SWC_CAN_INLINE
  ~Rangers() noexcept { }

  const Comm::client::ConnQueuesPtr queues;
  CachedRangers                     cache;

};



}} // namespace SWC::client




#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/service/rgr/Rangers.cc"
#endif


#endif // swcdb_db_client_rgr_Rangers_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_mngr_Managers_h
#define swcdb_db_client_mngr_Managers_h


#include "swcdb/db/client/service/mngr/ContextManager.h"
#include "swcdb/db/client/service/mngr/Groups.h"
#include "swcdb/core/comm/ClientConnQueues.h"


namespace SWC { namespace client {



class Managers  {
  public:

  SWC_CAN_INLINE
  Managers() noexcept : queues(nullptr), groups(nullptr) { }

  Managers(const Config::Settings& settings,
           Comm::IoContextPtr ioctx,
           const ContextManager::Ptr& mngr_ctx);

  //~Managers() { }

  bool put(const ClientsPtr& clients,
           const cid_t& cid, Comm::EndPoints& endpoints,
           const Comm::client::ConnQueue::ReqBase::Ptr& req);

  bool put_role_schemas(const ClientsPtr& clients,
                        Comm::EndPoints& endpoints,
                        const Comm::client::ConnQueue::ReqBase::Ptr& req);

  const Comm::client::ConnQueuesPtr queues;
  const Mngr::Groups::Ptr           groups;

};



}} // namespace SWC::client



#endif // swcdb_db_client_mngr_Managers_h

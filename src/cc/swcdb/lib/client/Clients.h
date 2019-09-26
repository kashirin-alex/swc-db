/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_Clients_h
#define swc_lib_client_Clients_h

#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/client/mngr/Groups.h"
#include "swcdb/lib/core/comm/SerializedClient.h"

#include "swcdb/lib/db/Protocol/Common/req/ConnQueue.h"
#include <memory>

namespace SWC { namespace client {

class ConnQueues;
typedef std::shared_ptr<ConnQueues> ConnQueuesPtr;

class Clients;
typedef std::shared_ptr<Clients> ClientsPtr;

class Clients : public std::enable_shared_from_this<Clients> {
  public:

  Clients(IOCtxPtr ioctx, const AppContextPtr app_ctx)
          : m_app_ctx(app_ctx),
            mngrs_groups(std::make_shared<Mngr::Groups>()->init()) {

    if(ioctx == nullptr){
      if(!Env::IoCtx::ok())
        Env::IoCtx::init(8);
      ioctx = Env::IoCtx::io()->shared();
    }

    mngr_service = std::make_shared<SerializedClient>(
      "MANAGER", ioctx, m_app_ctx
    );
    mngr = std::make_shared<ConnQueues>(
      mngr_service,
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.Mngr.connection.timeout"),
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.Mngr.connection.probes"),
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.Mngr.connection.keepalive")
    );

    rgr_service = std::make_shared<SerializedClient>(
      "RANGER", ioctx, m_app_ctx
    );
    rgr = std::make_shared<ConnQueues>(
      rgr_service,
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.Rgr.connection.timeout"),
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.Rgr.connection.probes"),
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.Rgr.connection.keepalive")
    );
  } 

  operator ClientsPtr(){
    return shared_from_this();
  }

  virtual ~Clients(){}
  

  const Mngr::GroupsPtr   mngrs_groups;
  ClientPtr               mngr_service = nullptr;
  ConnQueuesPtr           mngr = nullptr;
  
  ClientPtr               rgr_service   = nullptr;
  ConnQueuesPtr           rgr = nullptr;

  private:
  const AppContextPtr     m_app_ctx = nullptr;
};

} // namespace client 


namespace Env {
class Clients {
  public:

  static void init(client::ClientsPtr clients) {
    m_env = std::make_shared<Clients>(clients);
  }

  static client::ClientsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_clients;
  }

  Clients(client::ClientsPtr clients) : m_clients(clients) {}
  virtual ~Clients(){}

  private:
  client::ClientsPtr                      m_clients = nullptr;
  inline static std::shared_ptr<Clients>  m_env = nullptr;
};
}

}

#include "ConnQueues.h"

#endif // swc_lib_client_Clients_h
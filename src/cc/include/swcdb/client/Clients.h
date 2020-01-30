/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_Clients_h
#define swc_lib_client_Clients_h

#include <memory>

#include "swcdb/client/Settings.h"
#include "swcdb/client/AppContext.h"
#include "swcdb/client/mngr/Groups.h"
#include "swcdb/core/comm/SerializedClient.h"
#include "swcdb/db/Protocol/Common/req/ConnQueue.h"
#include "swcdb/client/rgr/Rangers.h"

namespace SWC { namespace client {

class Schemas;
typedef std::shared_ptr<Schemas> SchemasPtr;

class ConnQueues;
typedef std::shared_ptr<ConnQueues> ConnQueuesPtr;

class Clients final {
  public:

  typedef std::shared_ptr<Clients> Ptr;

  Clients(IOCtxPtr ioctx, const AppContext::Ptr app_ctx)
          : m_app_ctx(app_ctx),
            mngrs_groups(std::make_shared<Mngr::Groups>()->init()),
            rangers(Env::Config::settings()->get_ptr<gInt32t>(
              "swc.client.Rgr.range.res.expiry")) {

    if(ioctx == nullptr){
      if(!Env::IoCtx::ok())
        Env::IoCtx::init(8);
      ioctx = Env::IoCtx::io()->shared();
    }

    mngr_service = std::make_shared<Serialized>(
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

    rgr_service = std::make_shared<Serialized>(
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

    schemas = std::make_shared<Schemas>(
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.client.schema.expiry")
    );
  } 

  ~Clients(){}
  

  const Mngr::Groups::Ptr mngrs_groups;
  Serialized::Ptr         mngr_service = nullptr;
  ConnQueuesPtr           mngr = nullptr;
  
  Serialized::Ptr         rgr_service   = nullptr;
  ConnQueuesPtr           rgr = nullptr;

  SchemasPtr              schemas = nullptr;
  Rangers                 rangers;

  private:
  const AppContext::Ptr   m_app_ctx = nullptr;
};

} // namespace client 


namespace Env {
class Clients final {
  public:

  static void init(client::Clients::Ptr clients) {
    m_env = std::make_shared<Clients>(clients);
  }

  static client::Clients::Ptr get(){
    SWC_ASSERT(m_env != nullptr);
    return m_env->m_clients;
  }

  Clients(client::Clients::Ptr clients) : m_clients(clients) {}
  ~Clients(){}

  private:
  client::Clients::Ptr                    m_clients = nullptr;
  inline static std::shared_ptr<Clients>  m_env = nullptr;
};
}

}

#include "swcdb/client/ConnQueues.h"
#include "swcdb/client/Schemas.h"

#endif // swc_lib_client_Clients_h
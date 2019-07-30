/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_Clients_h
#define swc_lib_client_Clients_h

#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/client/mngr/Groups.h"
#include "swcdb/lib/core/comm/SerializedClient.h"

#include <memory>

namespace SWC { 
  

namespace client {

class Clients;
typedef std::shared_ptr<Clients> ClientsPtr;

class Clients : public std::enable_shared_from_this<Clients> {
  public:

  Clients(IOCtxPtr ioctx, AppContextPtr app_ctx)
          : m_app_ctx(app_ctx) {

    if(ioctx == nullptr){
      if(!EnvIoCtx::ok())
        EnvIoCtx::init(8);
      ioctx = EnvIoCtx::io()->shared();
    }

    mngrs_groups = std::make_shared<Mngr::Groups>()->init();

    mngr_service = std::make_shared<SerializedClient>(
      "RS-MANAGER", ioctx, m_app_ctx
    );
    rs_service = std::make_shared<SerializedClient>(
      "RANGESERVER", ioctx, m_app_ctx
    );

  }
  operator ClientsPtr(){
    return shared_from_this();
  }

  virtual ~Clients(){}
  

  Mngr::GroupsPtr mngrs_groups;
  ClientPtr       mngr_service = nullptr;
  ClientPtr       rs_service   = nullptr;

  private:
  AppContextPtr     m_app_ctx = nullptr;
};

} // namespace client 


class EnvClients;
typedef std::shared_ptr<EnvClients> EnvClientsPtr;

class EnvClients {
  
  public:

  static void init(client::ClientsPtr clients) {
    m_env = std::make_shared<EnvClients>(clients);
  }

  static client::ClientsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_clients;
  }

  EnvClients(client::ClientsPtr clients) : m_clients(clients) {}
  virtual ~EnvClients(){}

  private:
  client::ClientsPtr m_clients = nullptr;
  inline static EnvClientsPtr m_env = nullptr;
};

}

#endif // swc_lib_client_Clients_h
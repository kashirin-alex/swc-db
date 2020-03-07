/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_Clients_h
#define swc_lib_client_Clients_h

#include <memory>

#include "swcdb/core/comm/ClientConnQueues.h"
#include "swcdb/client/Settings.h"
#include "swcdb/client/AppContext.h"
#include "swcdb/client/Schemas.h"
#include "swcdb/client/mngr/Groups.h"
#include "swcdb/client/rgr/Rangers.h"

namespace SWC { namespace client {

class Clients final {
  public:

  typedef std::shared_ptr<Clients> Ptr;

  Clients(IOCtxPtr ioctx, const AppContext::Ptr app_ctx);

  ~Clients();
  
  const Mngr::Groups::Ptr mngrs_groups;
  Serialized::Ptr         mngr_service = nullptr;
  ConnQueuesPtr           mngr = nullptr;
  
  Serialized::Ptr         rgr_service   = nullptr;
  ConnQueuesPtr           rgr = nullptr;

  Schemas::Ptr            schemas = nullptr;
  Rangers                 rangers;

  private:
  const AppContext::Ptr   m_app_ctx = nullptr;
};

} // namespace client 


namespace Env {
class Clients final {
  public:

  static void init(client::Clients::Ptr clients);

  static client::Clients::Ptr get();

  static const Clients& ref();

  const Property::V_GINT32::Ptr      cfg_send_buff_sz;
  const Property::V_GUINT8::Ptr      cfg_send_ahead;
  const Property::V_GINT32::Ptr      cfg_send_timeout;
  const Property::V_GINT32::Ptr      cfg_send_timeout_ratio;

  const Property::V_GINT32::Ptr      cfg_recv_buff_sz;
  const Property::V_GUINT8::Ptr      cfg_recv_ahead;
  const Property::V_GINT32::Ptr      cfg_recv_timeout;

  Clients(client::Clients::Ptr clients) ;

  ~Clients();

  private:
  client::Clients::Ptr                    m_clients = nullptr;
  inline static std::shared_ptr<Clients>  m_env = nullptr;
};
}

}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/client/Clients.cc"
#include "swcdb/client/Schemas.cc"
#endif 


#endif // swc_lib_client_Clients_h
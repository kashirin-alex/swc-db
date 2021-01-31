/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Clients_h
#define swcdb_db_client_Clients_h

#include <memory>

#include "swcdb/core/comm/ClientConnQueues.h"
#include "swcdb/db/client/Settings.h"
#include "swcdb/db/client/ContextManager.h"
#include "swcdb/db/client/ContextRanger.h"
#include "swcdb/db/client/Schemas.h"
#include "swcdb/db/client/mngr/Groups.h"
#include "swcdb/db/client/rgr/Rangers.h"

namespace SWC {


/**
 * @brief The SWC-DB Client C++ namespace 'SWC::client'
 *
 * \ingroup Database
 */
namespace client {


Comm::IoContextPtr default_io();


class Clients final {
  public:

  typedef std::shared_ptr<Clients> Ptr;

  Clients(Comm::IoContextPtr ioctx,
          const ContextManager::Ptr& mngr_ctx,
          const ContextRanger::Ptr& rgr_ctx);

  ~Clients();

  const Mngr::Groups::Ptr         mngrs_groups;
  Comm::client::ConnQueuesPtr     mngr;
  Comm::client::ConnQueuesPtr     rgr;
  Schemas::Ptr                    schemas;
  Rangers                         rangers;

};

} // namespace client



namespace Env {


class Clients final {
  public:

  static void init(const client::Clients::Ptr& clients);

  static client::Clients::Ptr get();

  static const Clients& ref() noexcept;

  const SWC::Config::Property::V_GINT32::Ptr      cfg_send_buff_sz;
  const SWC::Config::Property::V_GUINT8::Ptr      cfg_send_ahead;
  const SWC::Config::Property::V_GINT32::Ptr      cfg_send_timeout;
  const SWC::Config::Property::V_GINT32::Ptr      cfg_send_timeout_ratio;

  const SWC::Config::Property::V_GINT32::Ptr      cfg_recv_buff_sz;
  const SWC::Config::Property::V_GUINT8::Ptr      cfg_recv_ahead;
  const SWC::Config::Property::V_GINT32::Ptr      cfg_recv_timeout;

  Clients(const client::Clients::Ptr& clients);

  ~Clients();

  private:
  client::Clients::Ptr                    m_clients = nullptr;
  inline static std::shared_ptr<Clients>  m_env = nullptr;

};


} // namespace Env

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Clients.cc"
#include "swcdb/db/client/Schemas.cc"
#endif


#endif // swcdb_db_client_Clients_h

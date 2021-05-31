/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Clients_h
#define swcdb_db_client_Clients_h


#include "swcdb/core/comm/ClientConnQueues.h"
#include "swcdb/db/client/Settings.h"

#include "swcdb/db/client/service/mngr/Managers.h"
#include "swcdb/db/client/service/rgr/Rangers.h"
#include "swcdb/db/client/service/bkr/Brokers.h"



namespace SWC {


/**
 * @brief The SWC-DB Client C++ namespace 'SWC::client'
 *
 * \ingroup Database
 */
namespace client {

class Clients;

}}

#include "swcdb/db/client/Schemas.h"



namespace SWC { namespace client {



class Clients : public std::enable_shared_from_this<Clients> {
  protected:

  Clients(const Config::Settings& settings,
          const Comm::IoContextPtr& io_ctx,
          const ContextManager::Ptr& mngr_ctx,
          const ContextRanger::Ptr& rgr_ctx,
          const ContextBroker::Ptr& bkr_ctx);

  Clients(const Config::Settings& settings,
          const Comm::IoContextPtr& io_ctx,
          const ContextManager::Ptr& mngr_ctx,
          const ContextRanger::Ptr& rgr_ctx);

  Clients(const Config::Settings& settings,
          const Comm::IoContextPtr& io_ctx,
          const ContextBroker::Ptr& bkr_ctx);

  public:
  typedef std::shared_ptr<Clients> Ptr;

  enum Flag : uint8_t {
    DEFAULT = 0x01,
    BROKER  = 0x02,
    SCHEMA  = 0x04,
  };

  static Ptr make(const Config::Settings& settings,
                  const Comm::IoContextPtr& io_ctx,
                  const ContextManager::Ptr& mngr_ctx,
                  const ContextRanger::Ptr& rgr_ctx,
                  const ContextBroker::Ptr& bkr_ctx);

  static Ptr make(const Config::Settings& settings,
                  const Comm::IoContextPtr& io_ctx,
                  const ContextManager::Ptr& mngr_ctx,
                  const ContextRanger::Ptr& rgr_ctx);

  static Ptr make(const Config::Settings& settings,
                  const Comm::IoContextPtr& io_ctx,
                  const ContextBroker::Ptr& bkr_ctx);

  //~Clients() { }

  Ptr shared() {
    return shared_from_this();
  }

  Ptr init() {
    if(has_brokers()) {
      brokers.cfg_hosts->set_cb_on_chg(
        [ptr=shared()]() noexcept { ptr->brokers.on_cfg_update(); }
      );
    }
    return shared();
  }

  void stop_services();

  void stop_io() {
    io_ctx->stop();
  }

  void stop();

  bool stopping() const noexcept {
    return !running;
  }

  void set_flags(uint8_t _flags) noexcept {
    flags.store(_flags);
  }

  void set_flags__schemas_via_default() noexcept {
    set_flags(Flag::DEFAULT | Flag::BROKER);
  }

  Comm::IoContextPtr& get_io() noexcept {
    return io_ctx;
  }

  bool has_brokers() noexcept {
    return bool(brokers.queues);
  }

  DB::Schema::Ptr get_schema(int& err, cid_t cid) {
    return schemas.get(err, cid);
  }

  DB::Schema::Ptr get_schema(int& err, const std::string& name) {
    return schemas.get(err, name);
  }

  void get_schema(int& err, const std::vector<DB::Schemas::Pattern>& patterns,
                  std::vector<DB::Schema::Ptr>& _schemas) {
    schemas.get(err, patterns, _schemas);
  }

  std::vector<DB::Schema::Ptr>
  get_schema(int& err, const std::vector<DB::Schemas::Pattern>& patterns) {
    return schemas.get(err, patterns);
  }


  void rgr_cache_remove(const cid_t cid, const rid_t rid) {
    rangers.cache.remove(cid, rid);
  }

  bool rgr_cache_get(const cid_t cid, const rid_t rid,
                     Comm::EndPoints& endpoints) {
    return rangers.cache.get(cid, rid, endpoints);
  }

  void rgr_cache_set(const cid_t cid, const rid_t rid,
                     const Comm::EndPoints& endpoints) {
    rangers.cache.set(cid, rid, endpoints);
  }


  Comm::IoContextPtr get_mngr_io() {
    return managers.queues->service->io();
  }

  Comm::client::Host::Ptr get_mngr_queue(const Comm::EndPoints& endpoints) {
    return managers.queues->get(endpoints);
  }


  Comm::IoContextPtr get_rgr_io() {
    return rangers.queues->service->io();
  }

  Comm::client::Host::Ptr get_rgr_queue(const Comm::EndPoints& endpoints) {
    return rangers.queues->get(endpoints);
  }


  Comm::IoContextPtr get_bkr_io() {
    return brokers.queues->service->io();
  }

  Comm::client::Host::Ptr get_bkr_queue(const Comm::EndPoints& endpoints) {
    return brokers.queues->get(endpoints);
  }

  void get_mngr(const cid_t& cid, Comm::EndPoints& endpoints) {
    managers.groups->select(cid, endpoints);
  }

  void get_mngr(const uint8_t& role, Comm::EndPoints& endpoints) {
    managers.groups->select(role, endpoints);
  }

  void remove_mngr(const Comm::EndPoints& endpoints) {
    managers.groups->remove(endpoints);
  }


  Core::AtomicBool                       running;
  Core::Atomic<uint8_t>                  flags;

  const Config::Property::V_GINT32::Ptr  cfg_send_buff_sz;
  const Config::Property::V_GUINT8::Ptr  cfg_send_ahead;
  const Config::Property::V_GINT32::Ptr  cfg_send_timeout;
  const Config::Property::V_GINT32::Ptr  cfg_send_timeout_ratio;

  const Config::Property::V_GINT32::Ptr  cfg_recv_buff_sz;
  const Config::Property::V_GUINT8::Ptr  cfg_recv_ahead;
  const Config::Property::V_GINT32::Ptr  cfg_recv_timeout;

  Comm::IoContextPtr                     io_ctx;
  Schemas                                schemas;
  Managers                               managers;
  Rangers                                rangers;
  Brokers                                brokers;

};

} // namespace client



namespace Env {


class Clients final {
  public:

  static void init(const client::Clients::Ptr& clients);

  static client::Clients::Ptr& get() noexcept {
    return m_env->m_clients;
  }

  static const Clients& ref() noexcept {
    return *m_env.get();
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  Clients(const client::Clients::Ptr& clients) noexcept
          : m_clients(clients) {
  }

  //~Clients() { }

  private:
  client::Clients::Ptr                    m_clients = nullptr;
  inline static std::shared_ptr<Clients>  m_env = nullptr;

};


} // namespace Env

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Clients.cc"
#include "swcdb/db/client/Schemas.cc"
#include "swcdb/db/client/Query/Update/Handlers/Base.cc"
#include "swcdb/db/client/Query/Select/Handlers/Base.cc"
#endif


#endif // swcdb_db_client_Clients_h

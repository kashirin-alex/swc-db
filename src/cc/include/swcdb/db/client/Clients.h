/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Clients_h
#define swcdb_db_client_Clients_h


#include "swcdb/core/comm/ClientConnQueues.h"
#include "swcdb/db/client/Settings.h"

namespace SWC {


/**
 * @brief The SWC-DB Client C++ namespace 'SWC::client'
 *
 * \ingroup Database
 */
namespace client {

class Clients;
typedef std::shared_ptr<Clients> ClientsPtr;

}}

#include "swcdb/db/client/service/mngr/Managers.h"
#include "swcdb/db/client/service/rgr/Rangers.h"
#include "swcdb/db/client/service/bkr/Brokers.h"
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
  using Ptr = ClientsPtr;

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

  ~Clients() noexcept;

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

  SWC_CAN_INLINE
  bool stopping() const noexcept {
    return !running;
  }

  SWC_CAN_INLINE
  void set_flags(uint8_t _flags) noexcept {
    flags.store(_flags);
  }

  SWC_CAN_INLINE
  void set_flags__schemas_via_default() noexcept {
    set_flags(Flag::DEFAULT | Flag::BROKER);
  }

  SWC_CAN_INLINE
  Comm::IoContextPtr& get_io() noexcept {
    return io_ctx;
  }

  SWC_CAN_INLINE
  bool has_brokers() noexcept {
    return bool(brokers.queues);
  }

  SWC_CAN_INLINE
  DB::Schema::Ptr get_schema(int& err, cid_t cid,
                             uint32_t timeout=300000) {
    return schemas.get(err, cid, timeout);
  }

  SWC_CAN_INLINE
  DB::Schema::Ptr get_schema(int& err, const std::string& name,
                             uint32_t timeout=300000) {
    return schemas.get(err, name, timeout);
  }

  SWC_CAN_INLINE
  void get_schema(int& err, const DB::Schemas::SelectorPatterns& patterns,
                  DB::SchemasVec& _schemas,
                  uint32_t timeout=300000) {
    schemas.get(err, patterns, _schemas, timeout);
  }

  SWC_CAN_INLINE
  DB::SchemasVec
  get_schema(int& err, const DB::Schemas::SelectorPatterns& patterns,
             uint32_t timeout=300000) {
    return schemas.get(err, patterns, timeout);
  }


  SWC_CAN_INLINE
  void rgr_cache_remove(const cid_t cid, const rid_t rid) {
    rangers.cache.remove(cid, rid);
  }

  SWC_CAN_INLINE
  bool rgr_cache_get(const cid_t cid, const rid_t rid,
                     Comm::EndPoints& endpoints) {
    return rangers.cache.get(cid, rid, endpoints);
  }

  SWC_CAN_INLINE
  void rgr_cache_set(const cid_t cid, const rid_t rid,
                     const Comm::EndPoints& endpoints) {
    rangers.cache.set(cid, rid, endpoints);
  }


  SWC_CAN_INLINE
  void mngr_cache_remove_master(const cid_t cid, const rid_t rid) {
    managers.master_ranges_cache.remove(cid, rid);
  }

  SWC_CAN_INLINE
  void mngr_cache_set_master(const cid_t cid, const rid_t rid,
                             const DB::Cell::Key& range_begin,
                             const DB::Cell::Key& range_end,
                             const Comm::EndPoints& endpoints,
                             const int64_t revision) {
    managers.master_ranges_cache.set(
      cid, rid, range_begin, range_end, endpoints, revision);
  }

  SWC_CAN_INLINE
  bool mngr_cache_get_read_master(const cid_t cid,
                                  const DB::Cell::Key& range_begin,
                                  const DB::Cell::Key& range_end,
                                  rid_t& rid,
                                  DB::Cell::Key& offset,
                                  bool& is_end,
                                  Comm::EndPoints& endpoints,
                                  int64_t& revision) {
    return managers.master_ranges_cache.get_read(
      cid, range_begin, range_end, rid, offset, is_end, endpoints, revision);
  }

  SWC_CAN_INLINE
  bool mngr_cache_get_write_master(const cid_t cid,
                                   const DB::Cell::Key& key,
                                   rid_t& rid,
                                   DB::Cell::Key& key_end,
                                   Comm::EndPoints& endpoints,
                                   int64_t& revision) {
    return managers.master_ranges_cache.get_write(
      cid, key, rid, key_end, endpoints, revision);
  }


  SWC_CAN_INLINE
  Comm::IoContextPtr get_mngr_io() {
    return managers.queues->service->io();
  }

  SWC_CAN_INLINE
  Comm::client::Host::Ptr get_mngr_queue(const Comm::EndPoints& endpoints) {
    return managers.queues->get(endpoints);
  }


  SWC_CAN_INLINE
  Comm::IoContextPtr get_rgr_io() {
    return rangers.queues->service->io();
  }

  SWC_CAN_INLINE
  Comm::client::Host::Ptr get_rgr_queue(const Comm::EndPoints& endpoints) {
    return rangers.queues->get(endpoints);
  }


  SWC_CAN_INLINE
  Comm::IoContextPtr get_bkr_io() {
    return brokers.queues->service->io();
  }

  SWC_CAN_INLINE
  Comm::client::Host::Ptr get_bkr_queue(const Comm::EndPoints& endpoints) {
    return brokers.queues->get(endpoints);
  }

  SWC_CAN_INLINE
  void get_mngr(const cid_t& cid, Comm::EndPoints& endpoints) {
    managers.groups->select(cid, endpoints);
  }

  SWC_CAN_INLINE
  void get_mngr(const uint8_t& role, Comm::EndPoints& endpoints) {
    managers.groups->select(role, endpoints);
  }

  SWC_CAN_INLINE
  void remove_mngr(const Comm::EndPoints& endpoints) {
    managers.groups->remove(endpoints);
  }


  Core::AtomicBool                            running;
  Core::Atomic<uint8_t>                       flags;

  const Config::Property::Value_int32_g::Ptr  cfg_send_buff_sz;
  const Config::Property::Value_uint8_g::Ptr  cfg_send_ahead;
  const Config::Property::Value_int32_g::Ptr  cfg_send_timeout;
  const Config::Property::Value_int32_g::Ptr  cfg_send_timeout_ratio;

  const Config::Property::Value_int32_g::Ptr  cfg_recv_buff_sz;
  const Config::Property::Value_uint8_g::Ptr  cfg_recv_ahead;
  const Config::Property::Value_int32_g::Ptr  cfg_recv_timeout;

  Comm::IoContextPtr                          io_ctx;
  Schemas                                     schemas;
  Managers                                    managers;
  Rangers                                     rangers;
  Brokers                                     brokers;

};

} // namespace client



namespace Env {


class Clients final {
  public:

  static void init(const client::Clients::Ptr& clients);

  SWC_CAN_INLINE
  static client::Clients::Ptr& get() noexcept {
    return m_env->m_clients;
  }

  SWC_CAN_INLINE
  static const Clients& ref() noexcept {
    return *m_env.get();
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  Clients(const client::Clients::Ptr& clients) noexcept
          : m_clients(clients) {
  }

  ~Clients() noexcept { }

  private:
  client::Clients::Ptr                    m_clients = nullptr;
  inline static std::shared_ptr<Clients>  m_env = nullptr;

};


} // namespace Env

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Clients.cc"
#include "swcdb/db/client/Schemas.cc"
#include "swcdb/db/client/service/mngr/Managers.cc"
#include "swcdb/db/client/Query/Update/Handlers/Base.cc"
#include "swcdb/db/client/Query/Select/Handlers/Base.cc"
#endif


#endif // swcdb_db_client_Clients_h

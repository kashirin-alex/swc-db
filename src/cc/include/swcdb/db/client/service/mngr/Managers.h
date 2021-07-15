/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_mngr_Managers_h
#define swcdb_db_client_mngr_Managers_h


#include "swcdb/db/client/service/mngr/ContextManager.h"
#include "swcdb/db/client/service/mngr/Groups.h"
#include "swcdb/core/comm/ClientConnQueues.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Types/SystemColumn.h"


namespace SWC { namespace client {



class Managers  {
  public:

  enum CACHE_APPLY_KEY : uint8_t {
    BEGIN = 0x00,
    END   = 0x01
  };


  private:

  class MasterRangesCache final {


    struct Range final {
      int64_t         ts;
      rid_t           rid;
      DB::Cell::Key   key_begin;
      DB::Cell::Key   key_end;
      Comm::EndPoints endpoints;
      int64_t         revision;
      SWC_CAN_INLINE
      Range(const int64_t ts,
            const rid_t rid,
            const DB::Cell::Key& range_begin,
            const DB::Cell::Key& range_end,
            const Comm::EndPoints& endpoints,
            const int64_t revision)
            : ts(ts), rid(rid), key_begin(range_begin), key_end(range_end),
              endpoints(endpoints), revision(revision) {
      }
      SWC_CAN_INLINE
      Range(Range&& other) noexcept
            : ts(other.ts),
              rid(other.rid),
              key_begin(std::move(other.key_begin)),
              key_end(std::move(other.key_end)),
              endpoints(std::move(other.endpoints)),
              revision(other.revision) {
      }
      SWC_CAN_INLINE
      void operator=(Range&& other) noexcept {
        ts = other.ts;
        rid = other.rid;
        key_begin = std::move(other.key_begin);
        key_end = std::move(other.key_end);
        endpoints = std::move(other.endpoints);
        revision = other.revision;
      }
      SWC_CAN_INLINE
      void change(const int64_t _ts,
                  const rid_t _rid,
                  const DB::Cell::Key& _range_begin,
                  const DB::Cell::Key& _range_end,
                  const Comm::EndPoints& _endpoints,
                  const int64_t _revision) {
        ts = _ts;
        rid = _rid;
        key_begin.copy(_range_begin);
        key_end.copy(_range_end);
        endpoints = _endpoints;
        revision = _revision;
      }
    };


    class Column final : private Core::Vector<Range> {

      Config::Property::V_GINT32::Ptr expiry_ms;
      DB::Types::KeySeq               key_seq;
      mutable Core::MutexSptd         m_mutex;

      public:

      void init(DB::Types::KeySeq _key_seq,
                Config::Property::V_GINT32::Ptr _expiry_ms) noexcept {
        key_seq = _key_seq;
        expiry_ms = _expiry_ms;
      }

      void clear_expired() noexcept;

      void remove(const rid_t rid) noexcept;

      void set(const rid_t rid,
               const DB::Cell::Key& range_begin,
               const DB::Cell::Key& range_end,
               const Comm::EndPoints& endpoints,
               const int64_t revision);

      bool get(const DB::Cell::Key& range_begin,
               const DB::Cell::Key& range_end,
               CACHE_APPLY_KEY kind,
               rid_t& rid,
               DB::Cell::Key& apply,
               Comm::EndPoints& endpoints, int64_t& revision);

    };


    public:

    SWC_CAN_INLINE
    MasterRangesCache() noexcept { }

    SWC_CAN_INLINE
    MasterRangesCache(const Config::Settings& settings) noexcept {
      Config::Property::V_GINT32::Ptr expiry_ms(
        settings.get<Config::Property::V_GINT32>(
          "swc.client.Mngr.range.master.expiry"));
      for(cid_t cid=1; cid<=DB::Types::SystemColumn::CID_MASTER_END; ++cid) {
        columns[cid - 1].init(
          DB::Types::SystemColumn::get_seq_type(cid),
          expiry_ms
        );
      }
    }

    void clear_expired() noexcept {
      for(auto& c : columns)
        c.clear_expired();
    }

    SWC_CAN_INLINE
    void remove(const cid_t cid, const rid_t rid) noexcept {
      columns[cid - 1].remove(rid);
    }

    SWC_CAN_INLINE
    void set(const cid_t cid, const rid_t rid,
             const DB::Cell::Key& range_begin,
             const DB::Cell::Key& range_end,
             const Comm::EndPoints& endpoints,
             const int64_t revision) {
      columns[cid - 1].set(
        rid, range_begin, range_end, endpoints, revision);
    }

    SWC_CAN_INLINE
    bool get(const cid_t cid,
             const DB::Cell::Key& range_begin,
             const DB::Cell::Key& range_end,
             CACHE_APPLY_KEY kind,
             rid_t& rid,
             DB::Cell::Key& apply,
             Comm::EndPoints& endpoints,
             int64_t& revision) {
      return columns[cid - 1].get(
        range_begin, range_end, kind, rid, apply, endpoints, revision);
    }

    private:
    Column columns[DB::Types::SystemColumn::CID_MASTER_END];

  };


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
  MasterRangesCache                 master_ranges_cache;

};



}} // namespace SWC::client



#endif // swcdb_db_client_mngr_Managers_h

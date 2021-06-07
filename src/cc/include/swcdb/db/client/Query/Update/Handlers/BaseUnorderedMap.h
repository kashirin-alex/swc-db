/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Update_Handlers_BaseUnorderedMap_h
#define swcdb_db_client_Query_Update_Handlers_BaseUnorderedMap_h


#include "swcdb/db/client/Query/Update/Handlers/BaseColumnMutable.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {


class BaseUnorderedMap
            : private std::unordered_map<cid_t, ColumnMutable::Ptr>,
              public Base {
  public:
  typedef std::shared_ptr<BaseUnorderedMap>             Ptr;
  typedef std::unordered_map<cid_t, ColumnMutable::Ptr> Map;

  SWC_CAN_INLINE
  BaseUnorderedMap(const Clients::Ptr& clients,
                   Clients::Flag executor=Clients::DEFAULT) noexcept
                  : Base(clients, executor) {
  }

  BaseUnorderedMap(const BaseUnorderedMap&) = delete;

  BaseUnorderedMap(const BaseUnorderedMap&&) = delete;

  BaseUnorderedMap& operator=(const BaseUnorderedMap&) = delete;

  virtual ~BaseUnorderedMap() { }


  virtual bool requires_commit() noexcept override;

  virtual bool empty() noexcept override;

  virtual size_t size_bytes() noexcept override;

  virtual void next(std::vector<Base::Column*>& cols) noexcept override;

  virtual Base::Column* next(cid_t cid) noexcept override;

  virtual void error(cid_t cid, int err) noexcept override;


  SWC_CAN_INLINE
  ColumnMutable::Ptr& create(const DB::Schema::Ptr& schema) {
    return create(
      schema->cid, schema->col_seq,
      schema->cell_versions, schema->cell_ttl,
      schema->col_type);
  }

  ColumnMutable::Ptr& create(const cid_t cid, DB::Types::KeySeq seq,
                             uint32_t versions, uint32_t ttl_secs,
                             DB::Types::Column type);

  bool exists(const cid_t cid) noexcept;

  void add(const cid_t cid, const DB::Cells::Cell& cell);

  ColumnMutable::Ptr get(const cid_t cid) noexcept;

  Base::Column* get_base_ptr(cid_t cid) noexcept;

  size_t size() noexcept;


  private:
  Core::MutexSptd                         m_mutex;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update/Handlers/BaseUnorderedMap.cc"
#endif


#endif // swcdb_db_client_Query_Update_Handlers_BaseUnorderedMap_h

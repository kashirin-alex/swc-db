/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Update_Handlers_BaseSingleColumn_h
#define swcdb_db_client_Query_Update_Handlers_BaseSingleColumn_h


#include "swcdb/db/client/Query/Update/Handlers/BaseColumnMutable.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {


class BaseSingleColumn : public Base {
  public:
  typedef std::shared_ptr<BaseSingleColumn> Ptr;

  using client::Query::Update::Handlers::Base::error;

  ColumnMutable column;

  SWC_CAN_INLINE
  BaseSingleColumn(const Clients::Ptr& a_clients,
                   const cid_t cid, DB::Types::KeySeq seq,
                   uint32_t versions, uint32_t ttl_secs,
                   DB::Types::Column type,
                   Clients::Flag a_executor=Clients::DEFAULT)
                  : Base(a_clients, a_executor),
                    column(cid, seq, versions, ttl_secs, type) {
  }

  SWC_CAN_INLINE
  BaseSingleColumn(const Clients::Ptr& a_clients,
                   const cid_t cid, DB::Types::KeySeq seq,
                   uint32_t versions, uint32_t ttl_secs,
                   DB::Types::Column type,
                   const StaticBuffer& buffer)
                  : Base(a_clients),
                    column(cid, seq, versions, ttl_secs, type, buffer) {
  }

  virtual ~BaseSingleColumn() noexcept { }


  virtual bool requires_commit() noexcept override {
    return !error() && !column.error() && !column.empty();
  }

  virtual bool empty() noexcept override {
    return column.empty();
  }

  virtual size_t size_bytes() noexcept override {
    return column.size_bytes();
  }

  virtual void next(Base::Colms& cols) noexcept override {
    cols.clear();
    if(requires_commit())
      cols.push_back(&column);
  }

  virtual Base::Column* next(cid_t cid) noexcept override{
    return cid == column.cid && requires_commit() ? &column : nullptr;
  }

  virtual void error(cid_t cid, int err) noexcept override {
    if(cid == column.cid)
      column.error(err);
    else
      error(Error::CLIENT_MISMATCHED_CID);
  }


  SWC_CAN_INLINE
  size_t size() noexcept {
    return column.size();
  }

};


}}}}}



#endif // swcdb_db_client_Query_Update_Handlers_BaseSingleColumn_h

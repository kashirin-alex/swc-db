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

  BaseSingleColumn(const Clients::Ptr& clients,
                   const cid_t cid, DB::Types::KeySeq seq,
                   uint32_t versions, uint32_t ttl_secs,
                   DB::Types::Column type,
                   Clients::Flag executor=Clients::DEFAULT)
                  : Base(clients, executor),
                    column(cid, seq, versions, ttl_secs, type) {
  }

  BaseSingleColumn(const Clients::Ptr& clients,
                   const cid_t cid, DB::Types::KeySeq seq,
                   uint32_t versions, uint32_t ttl_secs,
                   DB::Types::Column type,
                   const StaticBuffer& buffer)
                  : Base(clients),
                    column(cid, seq, versions, ttl_secs, type, buffer) {
  }

  BaseSingleColumn(const BaseSingleColumn&) = delete;

  BaseSingleColumn(const BaseSingleColumn&&) = delete;

  BaseSingleColumn& operator=(const BaseSingleColumn&) = delete;

  virtual ~BaseSingleColumn() { }


  virtual bool requires_commit() noexcept override {
    return !error() && !column.error() && !column.empty();
  }

  virtual bool empty() noexcept override {
    return column.empty();
  }

  virtual size_t size_bytes() noexcept override {
    return column.size_bytes();
  }

  virtual void next(std::vector<Base::Column*>& cols) noexcept override {
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


  size_t size() noexcept {
    return column.size();
  }

};


}}}}}



#endif // swcdb_db_client_Query_Update_Handlers_BaseSingleColumn_h

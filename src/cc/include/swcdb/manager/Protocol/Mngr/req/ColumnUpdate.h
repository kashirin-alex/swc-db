/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_req_ColumnUpdate_h
#define swcdb_manager_Protocol_mngr_req_ColumnUpdate_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/manager/Protocol/Mngr/params/ColumnUpdate.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {

class ColumnUpdate : public client::ConnQueue::ReqBase {
  public:

  ColumnUpdate(Params::ColumnMng::Function function,
               const DB::Schema::Ptr& schema,
               int err, uint64_t id)
              : client::ConnQueue::ReqBase(
                  Buffers::make(
                    Params::ColumnUpdate(function, schema, err, id),
                    0,
                    COLUMN_UPDATE, 60000
                  )) {
  }

  ColumnUpdate(cid_t cid_begin, cid_t cid_end,
               std::vector<cid_t>&& columns)
              : client::ConnQueue::ReqBase(
                  Buffers::make(
                    Params::ColumnUpdate(
                      Params::ColumnMng::Function::INTERNAL_EXPECT,
                      cid_begin, cid_end, std::move(columns)
                    ),
                    0,
                    COLUMN_UPDATE, 60000
                  )
                ) {
  }

  virtual ~ColumnUpdate() { }

  void handle_no_conn() override {
    Env::Mngr::role()->req_mngr_inchain(req());
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {
    if(ev->response_code())
      conn->do_close();
  }

};

}}}}}

#endif // swcdb_manager_Protocol_mngr_req_ColumnUpdate_h

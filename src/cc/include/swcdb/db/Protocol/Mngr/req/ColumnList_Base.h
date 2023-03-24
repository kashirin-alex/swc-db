/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnList_Base_h
#define swcdb_db_protocol_mngr_req_ColumnList_Base_h


#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnList_Base: public client::ConnQueue::ReqBase {
  public:

  SWC_CAN_INLINE
  ColumnList_Base(const Params::ColumnListReq& params,
                  const uint32_t timeout)
        : client::ConnQueue::ReqBase(
            Buffers::make(params, 0, COLUMN_LIST, timeout)),
          endpoints() {
  }

  virtual ~ColumnList_Base() noexcept { }

  void handle_no_conn() override;

  bool run() override {
    return get_clients()->managers.put_role_schemas(
      get_clients(), endpoints, req());
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  protected:

  virtual SWC::client::Clients::Ptr& get_clients() noexcept = 0;

  virtual void callback(int error, const Params::ColumnListRsp& rsp) = 0;

  private:

  EndPoints                     endpoints;
};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnList_Base.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnList_Base_h

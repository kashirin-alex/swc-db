/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_ColumnCompact_Base_h
#define swcdb_db_protocol_req_ColumnCompact_Base_h


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnCompact_Base: public client::ConnQueue::ReqBase {
  public:

  ColumnCompact_Base(const Params::ColumnCompactReq& params,
                     const uint32_t timeout);

  virtual ~ColumnCompact_Base() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  protected:

  virtual SWC::client::Clients::Ptr& get_clients() noexcept = 0;

  virtual void callback(const Params::ColumnCompactRsp& rsp) = 0;

  private:

  void clear_endpoints();

  const cid_t               cid;
  EndPoints                 endpoints;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Base.cc"
#endif

#endif // swcdb_db_protocol_req_ColumnCompact_Base_h

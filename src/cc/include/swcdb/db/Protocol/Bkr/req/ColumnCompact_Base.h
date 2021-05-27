/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnCompact_Base_h
#define swcdb_db_protocol_bkr_req_ColumnCompact_Base_h


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnCompact_Base: public client::ConnQueue::ReqBase {
  public:

  ColumnCompact_Base(const SWC::client::Clients::Ptr& clients,
                     const Mngr::Params::ColumnCompactReq& params,
                     const uint32_t timeout);

  virtual ~ColumnCompact_Base() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  protected:
  virtual void callback(const Mngr::Params::ColumnCompactRsp& rsp) = 0;

  private:
  SWC::client::Clients::Ptr       clients;
  SWC::client::Brokers::BrokerIdx _bkr_idx;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnCompact_Base.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnCompact_Base_h

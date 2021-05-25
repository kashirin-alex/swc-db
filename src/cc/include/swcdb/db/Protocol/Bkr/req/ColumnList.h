/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnList_h
#define swcdb_db_protocol_bkr_req_ColumnList_h


#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnList: public client::ConnQueue::ReqBase {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             int, const Mngr::Params::ColumnListRsp&)> Cb_t;

  static void request(const SWC::client::Clients::Ptr& clients,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnListReq& params,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  ColumnList(const SWC::client::Clients::Ptr& clients,
             const Mngr::Params::ColumnListReq& params, Cb_t&& cb,
             const uint32_t timeout);

  virtual ~ColumnList() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  SWC::client::Clients::Ptr clients;
  const Cb_t                cb;
  size_t                    bkr_idx;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnList.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnList_h

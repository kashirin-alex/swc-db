/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnList_h
#define swcdb_db_protocol_mngr_req_ColumnList_h


#include "swcdb/db/Protocol/Mngr/req/ColumnList_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnList: public ColumnList_Base {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             int, const Params::ColumnListRsp&)> Cb_t;

  static void request(const SWC::client::Clients::Ptr& clients,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnListReq& params,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  ColumnList(const SWC::client::Clients::Ptr& clients,
             const Params::ColumnListReq& params, Cb_t&& cb,
             const uint32_t timeout);

  virtual ~ColumnList() { }

  protected:
  virtual void callback(int err, const Params::ColumnListRsp& rsp) override;

  private:
  const Cb_t                cb;
};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnList.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnList_h

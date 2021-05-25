/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_ColumnCompact_h
#define swcdb_db_protocol_req_ColumnCompact_h


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnCompact: public ColumnCompact_Base {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const Params::ColumnCompactRsp&)> Cb_t;

  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnCompactReq& params,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  ColumnCompact(const SWC::client::Clients::Ptr& clients,
                const Params::ColumnCompactReq& params, Cb_t&& cb,
                const uint32_t timeout);

  virtual ~ColumnCompact() { }

  protected:
  void callback(const Params::ColumnCompactRsp& rsp) override;

  private:
  const Cb_t                cb;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.cc"
#endif

#endif // swcdb_db_protocol_req_ColumnCompact_h

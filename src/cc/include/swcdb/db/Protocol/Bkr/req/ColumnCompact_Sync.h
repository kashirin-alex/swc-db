/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnCompact_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnCompact_Sync_h


#include "swcdb/db/Protocol/Bkr/req/ColumnCompact_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnCompact_Sync: public ColumnCompact_Base {
  public:

  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, int& err, const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnCompactReq& params,
                      int& err, const uint32_t timeout = 10000);

  std::promise<void>       await;

  ColumnCompact_Sync(const SWC::client::Clients::Ptr& clients,
                     const Mngr::Params::ColumnCompactReq& params, int& err,
                     const uint32_t timeout);

  virtual ~ColumnCompact_Sync() { }

  protected:
  void callback(const Mngr::Params::ColumnCompactRsp& rsp) override;

  private:
  int&                     err;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnCompact_Sync.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnCompact_Sync_h

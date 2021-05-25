/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_ColumnCompact_Sync_h
#define swcdb_db_protocol_req_ColumnCompact_Sync_h


#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnCompact_Sync: public ColumnCompact_Base {
  public:

  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, int& err, const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnCompactReq& params,
                      int& err, const uint32_t timeout = 10000);

  std::promise<void>       await;

  ColumnCompact_Sync(const SWC::client::Clients::Ptr& clients,
                     const Params::ColumnCompactReq& params, int& err,
                     const uint32_t timeout);

  virtual ~ColumnCompact_Sync() { }

  protected:
  void callback(const Params::ColumnCompactRsp& rsp) override;

  private:
  int&                     err;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Sync.cc"
#endif

#endif // swcdb_db_protocol_req_ColumnCompact_Sync_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnList_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnList_Sync_h


#include "swcdb/db/Protocol/Bkr/req/ColumnList_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnList_Sync: public ColumnList_Base {
  public:

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnListReq& params,
                      int& err, std::vector<DB::Schema::Ptr>& schemas,
                      const uint32_t timeout = 10000);

  std::promise<void> await;

  ColumnList_Sync(const SWC::client::Clients::Ptr& clients,
                  const Mngr::Params::ColumnListReq& params,
                  int& err, std::vector<DB::Schema::Ptr>& schemas,
                  const uint32_t timeout);

  virtual ~ColumnList_Sync() { }

  protected:
  virtual void callback(int error,
                        const Mngr::Params::ColumnListRsp& rsp) override;

  private:
  int&                            err;
  std::vector<DB::Schema::Ptr>&   schemas;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnList_Sync.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnList_Sync_h

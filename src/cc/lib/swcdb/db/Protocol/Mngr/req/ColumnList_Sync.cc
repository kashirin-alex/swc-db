/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnList_Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnList_Sync::request(
                      const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnListReq& params,
                      int& err, std::vector<DB::Schema::Ptr>& schemas,
                      const uint32_t timeout) {
  auto req = std::make_shared<ColumnList_Sync>(
    clients, params, err, schemas, timeout);
  auto res = req->await.get_future();
  req->run();
  res.get();
}


ColumnList_Sync::ColumnList_Sync(
                      const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnListReq& params,
                      int& err, std::vector<DB::Schema::Ptr>& schemas,
                      const uint32_t timeout)
                      : ColumnList_Base(clients, params, timeout),
                        err(err), schemas(schemas) {
}

void ColumnList_Sync::callback(int error, const Params::ColumnListRsp& rsp) {
  err = error;
  schemas = std::move(rsp.schemas);
  await.set_value();
}


}}}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnGet_Sync::schema(const SWC::client::Clients::Ptr& clients,
                            int& err, const std::string& name,
                            DB::Schema::Ptr& _schema,
                            const uint32_t timeout) {
  request(
    clients,
    Params::ColumnGetReq(Params::ColumnGetReq::Flag::SCHEMA_BY_NAME, name),
    err, _schema, timeout
  );
}

SWC_SHOULD_INLINE
void ColumnGet_Sync::schema(const SWC::client::Clients::Ptr& clients,
                            int& err, cid_t cid,
                            DB::Schema::Ptr& _schema,
                            const uint32_t timeout) {
  request(
    clients,
    Params::ColumnGetReq(Params::ColumnGetReq::Flag::SCHEMA_BY_ID, cid),
    err, _schema, timeout
  );
}

SWC_SHOULD_INLINE
void ColumnGet_Sync::request(const SWC::client::Clients::Ptr& clients,
                             const Params::ColumnGetReq& params,
                             int& err, DB::Schema::Ptr& _schema,
                             const uint32_t timeout) {
  auto req = std::make_shared<ColumnGet_Sync>(
    clients, params, err, _schema, timeout);
  auto res = req->await.get_future();
  req->run();
  res.get();
}


ColumnGet_Sync::ColumnGet_Sync(
                const SWC::client::Clients::Ptr& clients,
                const Params::ColumnGetReq& params,
                int& err, DB::Schema::Ptr& _schema,
                const uint32_t timeout)
                : ColumnGet_Base(clients, params, timeout),
                  err(err), _schema(_schema) {
}

void ColumnGet_Sync::callback(int error, const Params::ColumnGetRsp& rsp) {
  err = error;
  _schema = std::move(rsp.schema);
  await.set_value();
}



}}}}}

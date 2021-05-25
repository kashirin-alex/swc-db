/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Sync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


SWC_SHOULD_INLINE
void ColumnMng_Sync::create(const SWC::client::Clients::Ptr& clients,
                            const DB::Schema::Ptr& schema,
                            int& err, const uint32_t timeout) {
  request(clients, Mngr::Params::ColumnMng::Function::CREATE, schema, err, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng_Sync::modify(const SWC::client::Clients::Ptr& clients,
                            const DB::Schema::Ptr& schema,
                            int& err, const uint32_t timeout) {
  request(clients, Mngr::Params::ColumnMng::Function::MODIFY, schema, err, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng_Sync::remove(const SWC::client::Clients::Ptr& clients,
                            const DB::Schema::Ptr& schema,
                            int& err, const uint32_t timeout) {
  request(clients, Mngr::Params::ColumnMng::Function::DELETE, schema, err, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng_Sync::request(const SWC::client::Clients::Ptr& clients,
                             Mngr::Params::ColumnMng::Function func,
                             const DB::Schema::Ptr& schema,
                             int& err, const uint32_t timeout) {
  request(clients, Mngr::Params::ColumnMng(func, schema), err, timeout);
}

SWC_SHOULD_INLINE
void ColumnMng_Sync::request(const SWC::client::Clients::Ptr& clients,
                             const Mngr::Params::ColumnMng& params,
                             int& err, const uint32_t timeout) {
  auto req = std::make_shared<ColumnMng_Sync>(clients, params, err, timeout);
  auto res = req->await.get_future();
  req->run();
  res.get();
}


ColumnMng_Sync::ColumnMng_Sync(
                    const SWC::client::Clients::Ptr& clients,
                    const Mngr::Params::ColumnMng& params,
                    int& err, const uint32_t timeout)
                    : ColumnMng_Base(clients, params, timeout),
                      err(err) {
}

void ColumnMng_Sync::callback(int error) {
  err = error;
  await.set_value();
}


}}}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/req/ColumnMng.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


SWC_SHOULD_INLINE
void ColumnMng::create(const SWC::client::Clients::Ptr& clients,
                       const DB::Schema::Ptr& schema,
                       ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Func::CREATE, schema, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::modify(const SWC::client::Clients::Ptr& clients,
                       const DB::Schema::Ptr& schema,
                       ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Func::MODIFY, schema, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::remove(const SWC::client::Clients::Ptr& clients,
                       const DB::Schema::Ptr& schema,
                       ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Func::DELETE, schema, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::request(const SWC::client::Clients::Ptr& clients,
                        ColumnMng::Func func, const DB::Schema::Ptr& schema,
                        ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  request(clients, Mngr::Params::ColumnMng(func, schema), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnMng::request(const SWC::client::Clients::Ptr& clients,
                        const Mngr::Params::ColumnMng& params,
                        ColumnMng::Cb_t&& cb, const uint32_t timeout) {
  std::make_shared<ColumnMng>(clients, params, std::move(cb), timeout)->run();
}


ColumnMng::ColumnMng(const SWC::client::Clients::Ptr& clients,
                     const Mngr::Params::ColumnMng& params,
                     ColumnMng::Cb_t&& cb, const uint32_t timeout)
                    : ColumnMng_Base(clients, params, timeout),
                      cb(std::move(cb)) {
}

void ColumnMng::callback(int error) {
  cb(req(), error);
}


}}}}}

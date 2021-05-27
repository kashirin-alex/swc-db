/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/ColumnGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


SWC_SHOULD_INLINE
void ColumnGet::schema(const SWC::client::Clients::Ptr& clients,
                       const std::string& name, ColumnGet::Cb_t&& cb,
                       const uint32_t timeout) {
  request(clients, Flag::SCHEMA_BY_NAME, name, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnGet::schema(const SWC::client::Clients::Ptr& clients,
                       cid_t cid, ColumnGet::Cb_t&& cb,
                       const uint32_t timeout) {
  request(clients, Flag::SCHEMA_BY_ID, cid, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnGet::cid(const SWC::client::Clients::Ptr& clients,
                    const std::string& name, ColumnGet::Cb_t&& cb,
                    const uint32_t timeout) {
  request(clients, Flag::ID_BY_NAME, name, std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnGet::request(const SWC::client::Clients::Ptr& clients,
                        ColumnGet::Flag flag, const std::string& name,
                        ColumnGet::Cb_t&& cb,
                        const uint32_t timeout) {
  std::make_shared<ColumnGet>(
    clients, Mngr::Params::ColumnGetReq(flag, name), std::move(cb), timeout
  )->run();
}

SWC_SHOULD_INLINE
void ColumnGet::request(const SWC::client::Clients::Ptr& clients,
                        ColumnGet::Flag flag, cid_t cid,
                        ColumnGet::Cb_t&& cb, const uint32_t timeout) {
  std::make_shared<ColumnGet>(
    clients, Mngr::Params::ColumnGetReq(flag, cid), std::move(cb), timeout
  )->run();
}


ColumnGet::ColumnGet(const SWC::client::Clients::Ptr& clients,
                     const Mngr::Params::ColumnGetReq& params,
                     ColumnGet::Cb_t&& cb,
                     const uint32_t timeout)
                    : ColumnGet_Base(clients, params, timeout),
                      cb(std::move(cb)) {
}

void ColumnGet::callback(int error, const Mngr::Params::ColumnGetRsp& rsp) {
  cb(req(), error, rsp);
}



}}}}}

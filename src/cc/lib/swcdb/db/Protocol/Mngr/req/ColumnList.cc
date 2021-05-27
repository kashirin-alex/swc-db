/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


SWC_SHOULD_INLINE
void ColumnList::request(const SWC::client::Clients::Ptr& clients,
                         ColumnList::Cb_t&& cb,
                         const uint32_t timeout) {
  request(
    clients, Params::ColumnListReq(), std::move(cb), timeout);
}

SWC_SHOULD_INLINE
void ColumnList::request(const SWC::client::Clients::Ptr& clients,
                         const Params::ColumnListReq& params,
                         ColumnList::Cb_t&& cb,
                         const uint32_t timeout) {
  std::make_shared<ColumnList>(
    clients, params, std::move(cb), timeout)->run();
}

ColumnList::ColumnList(const SWC::client::Clients::Ptr& clients,
                       const Params::ColumnListReq& params,
                       ColumnList::Cb_t&& cb,
                       const uint32_t timeout)
                      : ColumnList_Base(clients, params, timeout),
                        cb(std::move(cb)) {
}

void ColumnList::callback(int error, const Params::ColumnListRsp& rsp) {
  cb(req(), error, rsp);
}



}}}}}

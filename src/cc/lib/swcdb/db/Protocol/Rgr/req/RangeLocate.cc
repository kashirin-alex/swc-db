/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {

SWC_SHOULD_INLINE
void RangeLocate::request(const SWC::client::Clients::Ptr& clients,
                          const Params::RangeLocateReq& params,
                          const EndPoints& endpoints,
                          RangeLocate::Cb_t&& cb,
                          const uint32_t timeout) {
  std::make_shared<RangeLocate>(
    clients, params, endpoints, std::move(cb), timeout)->run();
}

RangeLocate::RangeLocate(const SWC::client::Clients::Ptr& clients,
                         const Params::RangeLocateReq& params,
                         const EndPoints& endpoints,
                         RangeLocate::Cb_t&& cb,
                         const uint32_t timeout)
                        : client::ConnQueue::ReqBase(
                            false,
                            Buffers::make(params, 0, RANGE_LOCATE, timeout)
                          ),
                          clients(clients),
                          endpoints(endpoints), cb(std::move(cb)) {
}

void RangeLocate::handle_no_conn() {
  cb(req(), Params::RangeLocateRsp(Error::COMM_NOT_CONNECTED));
}

bool RangeLocate::run() {
  clients->get_rgr_queue(endpoints)->put(req());
  return true;
}

void RangeLocate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  ev->type == Event::Type::DISCONNECT
    ? handle_no_conn()
    : cb(
        req(),
        Params::RangeLocateRsp(ev->error, ev->data.base, ev->data.size)
      );
}


}}}}}

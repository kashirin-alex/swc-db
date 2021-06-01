/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQueryUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


SWC_SHOULD_INLINE
void
RangeQueryUpdate::request(
        const SWC::client::Clients::Ptr& clients,
        const Params::RangeQueryUpdateReq& params,
        const DynamicBuffer::Ptr& buffer,
        const EndPoints& endpoints,
        RangeQueryUpdate::Cb_t&& cb,
        const uint32_t timeout) {
  std::make_shared<RangeQueryUpdate>(
    clients, params, buffer, endpoints, std::move(cb), timeout)->run();
}


RangeQueryUpdate::RangeQueryUpdate(
                const SWC::client::Clients::Ptr& clients,
                const Params::RangeQueryUpdateReq& params,
                const DynamicBuffer::Ptr& buffer,
                const EndPoints& endpoints,
                RangeQueryUpdate::Cb_t&& cb,
                const uint32_t timeout)
                : client::ConnQueue::ReqBase(false),
                  clients(clients), endpoints(endpoints), cb(std::move(cb)) {
  // timeout by buffer->fill() bytes ratio
  StaticBuffer snd_buf(buffer->base, buffer->fill(), false);
  cbp = Buffers::make(params, snd_buf, 0, RANGE_QUERY_UPDATE, timeout);
}

void RangeQueryUpdate::handle_no_conn() {
  cb(req(), Params::RangeQueryUpdateRsp(Error::COMM_NOT_CONNECTED));
}

bool RangeQueryUpdate::run() {
  clients->get_rgr_queue(endpoints)->put(req());
  return true;
}

void RangeQueryUpdate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  ev->type == Event::Type::DISCONNECT
    ? handle_no_conn()
    : cb(
        req(),
        Params::RangeQueryUpdateRsp(ev->error, ev->data.base, ev->data.size)
      );
}

}}}}}

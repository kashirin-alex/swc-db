/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Bkr/req/CellsUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


SWC_SHOULD_INLINE
void
CellsUpdate::request(
        const SWC::client::Clients::Ptr& clients,
        const EndPoints& endpoints,
        cid_t cid,
        const DynamicBuffer::Ptr& buffer,
        CellsUpdate::Cb_t&& cb,
        const uint32_t timeout) {
  StaticBuffer snd_buf(buffer->base, buffer->fill(), false);
  std::make_shared<CellsUpdate>(
    clients,
    endpoints,
    Buffers::make(
      Params::CellsUpdateReq(cid), snd_buf, 0, CELLS_UPDATE, timeout),
    std::move(cb)
  )->run();
}


CellsUpdate::CellsUpdate(
        const SWC::client::Clients::Ptr& clients,
        const EndPoints& endpoints,
        Buffers::Ptr&& cbp,
        CellsUpdate::Cb_t&& cb)
        : client::ConnQueue::ReqBase(std::move(cbp)),
          clients(clients),
          endpoints(endpoints),
          cb(std::move(cb)) {
}

void CellsUpdate::handle_no_conn() {
  cb(req(), Params::CellsUpdateRsp(Error::COMM_NOT_CONNECTED));
}

bool CellsUpdate::run() {
  clients->get_bkr_queue(endpoints)->put(req());
  return true;
}

void CellsUpdate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Params::CellsUpdateRsp rsp_params(ev->error);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      rsp_params.err = e.code();
    }
  }

  cb(req(), rsp_params);
}

}}}}}

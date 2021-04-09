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
        const Params::RangeQueryUpdateReq& params, 
        const DynamicBuffer::Ptr& buffer,
        const EndPoints& endpoints, 
        RangeQueryUpdate::Cb_t&& cb,
        const uint32_t timeout) {
  std::make_shared<RangeQueryUpdate>(
    params, buffer, endpoints, std::move(cb), timeout)->run();
}


RangeQueryUpdate::RangeQueryUpdate(
                const Params::RangeQueryUpdateReq& params,
                const DynamicBuffer::Ptr& buffer, 
                const EndPoints& endpoints,
                RangeQueryUpdate::Cb_t&& cb, 
                const uint32_t timeout) 
                : client::ConnQueue::ReqBase(false), 
                  endpoints(endpoints), cb(std::move(cb)) {
  // timeout by buffer->fill() bytes ratio
  StaticBuffer snd_buf(buffer->base, buffer->fill(), false);
  cbp = Buffers::make(params, snd_buf, 0, RANGE_QUERY_UPDATE, timeout);
}

RangeQueryUpdate::~RangeQueryUpdate() { }

void RangeQueryUpdate::handle_no_conn() {
  cb(req(), Params::RangeQueryUpdateRsp(Error::COMM_NOT_CONNECTED));
}

bool RangeQueryUpdate::run() {
  Env::Clients::get()->rgr->get(endpoints)->put(req());
  return true;
}

void RangeQueryUpdate::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::RangeQueryUpdateRsp rsp_params(ev->error);
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

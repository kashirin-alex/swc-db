
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQueryUpdate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


SWC_SHOULD_INLINE
void 
RangeQueryUpdate::request(
        const Params::RangeQueryUpdateReq& params, 
        const DynamicBuffer::Ptr& buffer,
        const EndPoints& endpoints, 
        const RangeQueryUpdate::Cb_t& cb, 
        const uint32_t timeout) {
  std::make_shared<RangeQueryUpdate>(params, buffer, endpoints, cb, timeout)
    ->run();
}


RangeQueryUpdate::RangeQueryUpdate(
                const Params::RangeQueryUpdateReq& params,
                const DynamicBuffer::Ptr& buffer, 
                const EndPoints& endpoints,
                const RangeQueryUpdate::Cb_t& cb, 
                const uint32_t timeout) 
                : client::ConnQueue::ReqBase(false), 
                  endpoints(endpoints), cb(cb) {
  // timeout by buffer->fill() bytes ratio
    StaticBuffer snd_buf(buffer->base, buffer->fill(), false);
  cbp = CommBuf::make(params, snd_buf);
  cbp->header.set(RANGE_QUERY_UPDATE, timeout);
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
  
  if(ev->type == Event::Type::DISCONNECT) {
    handle_no_conn();
    return;
  }

  Params::RangeQueryUpdateRsp rsp_params;
  if(ev->type == Event::Type::ERROR) {
    rsp_params.err = ev->error;
    cb(req(), rsp_params);
    return;
  }

  try{
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    rsp_params.decode(&ptr, &remain);
  } catch (Exception &e) {
    SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    rsp_params.err = e.code();
  }
  cb(req(), rsp_params);
}

}}}}


/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQuerySelect.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {

  
SWC_SHOULD_INLINE
void 
RangeQuerySelect::request(const Params::RangeQuerySelectReq& params,
                          const EndPoints& endpoints, 
                          const RangeQuerySelect::Cb_t& cb,
                          const uint32_t timeout) {
  std::make_shared<RangeQuerySelect>(params, endpoints, cb, timeout)->run();
}

RangeQuerySelect::RangeQuerySelect(const Params::RangeQuerySelectReq& params,
                                   const EndPoints& endpoints, 
                                   const RangeQuerySelect::Cb_t& cb, 
                                   const uint32_t timeout) 
                                  : client::ConnQueue::ReqBase(false), 
                                    endpoints(endpoints), cb(cb) {
  cbp = Buffers::make(params);
  cbp->header.set(RANGE_QUERY_SELECT, timeout);
}

RangeQuerySelect::~RangeQuerySelect() { }

void RangeQuerySelect::handle_no_conn() {
  cb(req(), Params::RangeQuerySelectRsp(Error::COMM_NOT_CONNECTED));
}

bool RangeQuerySelect::run() {
  Env::Clients::get()->rgr->get(endpoints)->put(req());
  return true;
}

void RangeQuerySelect::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::RangeQuerySelectRsp rsp_params(ev->error, ev->data_ext);
  if(!rsp_params.err) {
    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      rsp_params.err = e.code();
    }
  }

  cb(req(), rsp_params);
}

}}}}}

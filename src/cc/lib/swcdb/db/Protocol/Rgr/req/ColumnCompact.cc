
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Rgr/req/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {
  

ColumnCompact::ColumnCompact(cid_t cid) 
              : client::ConnQueue::ReqBase(false) {
  cbp = Buffers::make(Params::ColumnCompactReq(cid));
  cbp->header.set(COLUMN_COMPACT, 60000);
}

ColumnCompact::~ColumnCompact() { }

void ColumnCompact::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  if(ev->type == Event::Type::DISCONNECT)
    return handle_no_conn();

  Params::ColumnCompactRsp rsp_params(ev->error);
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

  if(rsp_params.err)
    request_again();
}

void ColumnCompact::handle_no_conn() { 
}



}}}}}

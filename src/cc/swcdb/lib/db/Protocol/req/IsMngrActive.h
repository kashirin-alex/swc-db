
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_IsMngrActive_h
#define swc_lib_db_protocol_req_IsMngrActive_h

#include "swcdb/lib/core/comm/DispatchHandler.h"

#include "swcdb/lib/db/Protocol/params/ReqIsMngrActive.h"

namespace SWC {
namespace Protocol {
namespace Req {

class IsMngrActive : public DispatchHandler {
  public:

  IsMngrActive(client::ClientConPtr conn, size_t begin, size_t end){

    Protocol::Params::ReqIsMngrActive params(begin, end);
    CommHeader header(Protocol::Command::MNGR_REQ_IS_MNGR_ACTIVE, 5000);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());

    conn->send_request(cbp, nullptr);

  }
  
};

}}}

#endif // swc_lib_db_protocol_req_IsMngrActive_h

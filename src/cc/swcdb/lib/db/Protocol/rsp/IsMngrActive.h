
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rsp_IsMngrActive_h
#define swc_lib_db_protocol_rsp_IsMngrActive_h

#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/db/Protocol/params/RspIsMngrActive.h"

namespace SWC {
namespace Protocol {
namespace Rsp {

class IsMngrActive {
  public:

  IsMngrActive(EndPoint endpoint,
               EventPtr ev, server::Mngr::RoleStatePtr role_state){
    try {
      const uint8_t *ptr = ev->payload;
      size_t remain = ev->payload_len;

      Protocol::Params::RspIsMngrActive params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);
        
      role_state->apply_state(endpoint, params.active);

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }
    
};

}}}

#endif // swc_lib_db_protocol_rsp_IsMngrActive_h

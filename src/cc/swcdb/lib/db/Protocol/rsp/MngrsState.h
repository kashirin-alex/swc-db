
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rsp_MngrsState_h
#define swc_lib_db_protocol_rsp_MngrsState_h

#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/db/Protocol/params/MngrsState.h"

namespace SWC {
namespace Protocol {
namespace Rsp {

class MngrsState {
  public:

  MngrsState(EndPoint endpoint,
             EventPtr ev, server::Mngr::RoleStatePtr role_state){
    try {
      const uint8_t *ptr = ev->payload;
      size_t remain = ev->payload_len;

      Protocol::Params::MngrsState params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);
        
      role_state->apply_states(endpoint, params.states);

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }
    
};

}}}

#endif // swc_lib_db_protocol_rsp_IsMngrActive_h

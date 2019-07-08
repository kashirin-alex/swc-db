
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_ReqMngrsState_h
#define swc_db_protocol_params_ReqMngrsState_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class MngrsState : public Serializable {
  public:

    MngrsState() {}

    MngrsState(server::Mngr::HostStatuses states, uint64_t token) 
              : states(states), token(token){}

    server::Mngr::HostStatuses states;
    uint64_t token;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      size_t len = 12;
      for(auto h : states )
        len += h->encoded_length();
      return len;
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i32(bufp, states.size());
      Serialization::encode_i64(bufp, token);
      for(auto h : states )
        h->encode(bufp);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      size_t len = Serialization::decode_i32(bufp, remainp);
      token = Serialization::decode_i64(bufp, remainp);
      for(size_t i =0; i<len; i++){
        server::Mngr::HostStatusPtr host = 
          std::make_shared<server::Mngr::HostStatus>();
        host->decode(bufp, remainp);
        states.push_back(host);
      }
    }

  };
  

}}}

#endif // swc_db_protocol_params_ReqIsMngrActive_h

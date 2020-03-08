
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_Protocol_Mngr_Params_MngrState_h
#define swc_db_Protocol_Mngr_Params_MngrState_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {

  class MngrState : public Serializable {
  public:

    MngrState() {}

    MngrState(Manager::MngrsStatus states, 
              uint64_t token, const EndPoint& mngr_host) 
              : states(states), token(token), mngr_host(mngr_host){}

    Manager::MngrsStatus states;
    uint64_t token;
    EndPoint mngr_host;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      size_t len = 12 + Serialization::encoded_length(mngr_host);
      for(auto& h : states )
        len += h->encoded_length();
      return len;
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i32(bufp, states.size());
      Serialization::encode_i64(bufp, token);
      Serialization::encode(mngr_host, bufp);
      for(auto& h : states )
        h->encode(bufp);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      size_t len = Serialization::decode_i32(bufp, remainp);
      token = Serialization::decode_i64(bufp, remainp);
      mngr_host = Serialization::decode(bufp, remainp);
      states.clear();
      states.resize(len);
      for(size_t i =0; i<len; i++)
        (states[i] = std::make_shared<Manager::MngrStatus>())
          ->decode(bufp, remainp);
    }

  };
  

}}}}

#endif // swc_db_Protocol_Mngr_Params_MngrState_h

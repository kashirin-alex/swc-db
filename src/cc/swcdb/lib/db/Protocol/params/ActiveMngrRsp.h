
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_ActiveMngrRsp_h
#define swc_db_protocol_params_ActiveMngrRsp_h

#include "swcdb/lib/core/Serializable.h"
#include "HostEndPoints.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class ActiveMngrRsp : public Serializable {
  public:

    ActiveMngrRsp() {}

    ActiveMngrRsp(HostEndPoints* host) 
                 : host(host), available(host!=nullptr) { }

    HostEndPoints* host; 
    bool available;
  
  private:

    uint8_t encoding_version() const {
      return 1;
    }

    size_t encoded_length_internal() const {
      return available ? host->encoded_length()+1 : 1;
    }

    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_bool(bufp, available);
      if(available)
        host->encode(bufp);
    }

    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      available = Serialization::decode_bool(bufp, remainp);
      if(available) {
        host = new HostEndPoints();
        host->decode(bufp, remainp);
      }
    }

  };
  

}}}

#endif // swc_db_protocol_params_ActiveMngrRsp_h

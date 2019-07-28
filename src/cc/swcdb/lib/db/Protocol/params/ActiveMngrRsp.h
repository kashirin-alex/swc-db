
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

  class ActiveMngrRsp : public HostEndPoints {
  public:

    ActiveMngrRsp() {}

    ActiveMngrRsp(EndPoints endpoints) 
                 : HostEndPoints(endpoints), available(endpoints.size()>0) { }
    bool available;
  
  private:

    size_t encoded_length_internal() const {
      size_t len = 1;
      if(available)
        len += HostEndPoints::encoded_length_internal();
      return len;
    }

    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_bool(bufp, available);
      if(available) {
        HostEndPoints::encode_internal(bufp);
      }
    }

    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      available = Serialization::decode_bool(bufp, remainp);
      if(available) {
        HostEndPoints::decode_internal(version, bufp, remainp);
      }
    }

  };
  

}}}

#endif // swc_db_protocol_params_ActiveMngrRsp_h

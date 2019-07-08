
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_RspIsMngrActive_h
#define swc_db_protocol_params_RspIsMngrActive_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class RspIsMngrActive : public Serializable {
  public:

    RspIsMngrActive() {}

    RspIsMngrActive(bool active) : active(active){}

    bool active; 

  private:


    uint8_t encoding_version() const {
      return 1;
    }

    size_t encoded_length_internal() const {
      return 1;
    }

    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_bool(bufp, active);
    }

    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      active = (size_t)Serialization::decode_bool(bufp, remainp);
    }

  };
  

}}}

#endif // swc_db_protocol_params_RspIsMngrActive_h

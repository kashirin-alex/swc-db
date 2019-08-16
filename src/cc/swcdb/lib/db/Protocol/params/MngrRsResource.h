
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrRsResource_h
#define swc_db_protocol_params_MngrRsResource_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class MngrRsResource : public Serializable {
  public:

    MngrRsResource() {}

    MngrRsResource(int64_t token, int64_t rs_id, int32_t chg) 
                   : token(token), rs_id(rs_id), chg(chg) {}

    int64_t token;
    int64_t rs_id;
    int32_t chg;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      return 12+Serialization::encoded_length_vi64(rs_id);
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i64(bufp, token);
      Serialization::encode_vi64(bufp, rs_id);
      Serialization::encode_i32(bufp, chg);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      token = Serialization::decode_v64(bufp, remainp);
      rs_id = Serialization::decode_vi64(bufp, remainp);
      chg = Serialization::decode_v32(bufp, remainp);
    }

  };
  

}}}

#endif // swc_db_protocol_params_ReqIsMngrActive_h

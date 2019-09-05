
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrUpdateColumn_h
#define swc_db_protocol_params_MngrUpdateColumn_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

class MngrUpdateColumn : public Serializable {
  public:

  MngrUpdateColumn() {}

  MngrUpdateColumn(MngColumn::Function function, int64_t cid, int err) 
                  : function(function), cid(cid), err(err) {}

  const std::string to_string() {
    std::string s("MngrUpdateColumn-params:\n");
    s.append(" func=");
    s.append(std::to_string(function));
    s.append(" cid=");
    s.append(std::to_string(cid));
    s.append(" err=");
    s.append(std::to_string(err));
    s.append("\n");
    return s;
  }
  
  MngColumn::Function function;
  int64_t             cid;
  int                 err;

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return 1
          + Serialization::encoded_length_vi64(cid) 
          + Serialization::encoded_length_vi32(err);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)function);
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi32(bufp, err);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                      size_t *remainp) {
    function = (MngColumn::Function)Serialization::decode_i8(bufp, remainp);
    cid = Serialization::decode_vi64(bufp, remainp);
    err = Serialization::decode_vi32(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_ReqIsMngrActive_h

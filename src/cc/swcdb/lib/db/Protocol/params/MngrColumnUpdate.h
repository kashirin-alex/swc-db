
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrColumnUpdate_h
#define swc_db_protocol_params_MngrColumnUpdate_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

class MngrColumnUpdate : public Serializable {
  public:

  MngrColumnUpdate() {}

  MngrColumnUpdate(ColumnMng::Function function, DB::SchemaPtr schema, int err) 
                  : function(function), schema(schema), err(err) {}

  const std::string to_string() {
    std::string s("MngrColumnUpdate-params:\n");
    s.append(" func=");
    s.append(std::to_string(function));
    s.append(" ");
    s.append(schema->to_string());
    s.append(" err=");
    s.append(std::to_string(err));
    s.append("\n");
    return s;
  }
  
  ColumnMng::Function function;
  DB::SchemaPtr       schema;
  int                 err;

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return 1
          + schema->encoded_length() 
          + Serialization::encoded_length_vi32(err);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)function);
    schema->encode(bufp);
    Serialization::encode_vi32(bufp, err);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                      size_t *remainp) {
    function = (ColumnMng::Function)Serialization::decode_i8(bufp, remainp);
    schema = std::make_shared<DB::Schema>(bufp, remainp);
    err = Serialization::decode_vi32(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_ReqIsMngrActive_h

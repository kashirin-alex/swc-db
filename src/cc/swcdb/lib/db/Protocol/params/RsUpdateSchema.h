
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_RsUpdateSchema_h
#define swc_db_protocol_params_RsUpdateSchema_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

class RsUpdateSchema : public Serializable {
  public:

  RsUpdateSchema() {}
  RsUpdateSchema(DB::SchemaPtr schema) : schema(schema){}
             
  virtual ~RsUpdateSchema() {}

  DB::SchemaPtr schema;
  
  private:

  uint8_t encoding_version() const {
    return 1;
  }

  size_t encoded_length_internal() const {
    return schema->encoded_length();
  }
    
  void encode_internal(uint8_t **bufp) const {
    schema->encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    schema = std::make_shared<DB::Schema>(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_RsUpdateSchema_h

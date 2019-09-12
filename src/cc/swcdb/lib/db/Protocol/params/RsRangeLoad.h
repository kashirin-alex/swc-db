
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_RsRangeLoad_h
#define swc_db_protocol_params_RsRangeLoad_h

#include "ColRangeId.h"

namespace SWC {
namespace Protocol {
namespace Params {

class RsRangeLoad : public ColRangeId {
  public:

  RsRangeLoad() : schema(nullptr) {}
  RsRangeLoad(size_t cid, size_t rid, DB::SchemaPtr schema) 
              : ColRangeId(cid, rid), schema(schema){}
             
  virtual ~RsRangeLoad() {}

  DB::SchemaPtr schema;
  
  private:

  size_t encoded_length_internal() const {
    return ColRangeId::encoded_length_internal()
           + 1 + (schema!=nullptr ? schema->encoded_length() : 0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    ColRangeId::encode_internal(bufp);
    Serialization::encode_bool(bufp, schema!=nullptr);
    if(schema!=nullptr)
      schema->encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    ColRangeId::decode_internal(version, bufp, remainp);
    if(Serialization::decode_bool(bufp, remainp))
      schema = std::make_shared<DB::Schema>(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_RsRangeLoad_h

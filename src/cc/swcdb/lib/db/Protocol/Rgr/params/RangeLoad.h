
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_RangeLoad_h
#define swc_db_protocol_rgr_params_RangeLoad_h

#include "../../Common/params/ColRangeId.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


class RangeLoad : public Common::Params::ColRangeId {
  public:

  RangeLoad() : schema(nullptr) {}
  RangeLoad(size_t cid, size_t rid, DB::SchemaPtr schema) 
            : Common::Params::ColRangeId(cid, rid), schema(schema){}
             
  virtual ~RangeLoad() {}

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
  
class RangeLoaded : public Serializable {
  public:
  RangeLoaded() {}
  RangeLoaded(const DB::Cells::Interval& interval): interval(interval) {}
  virtual ~RangeLoaded(){}
  
  DB::Cells::Interval interval;
  
  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return interval.encoded_length();
  }
    
  void encode_internal(uint8_t **bufp) const {
    interval.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    interval.decode(bufp, remainp);
  }

};

}}}}

#endif // swc_db_protocol_rgr_params_RangeLoad_h

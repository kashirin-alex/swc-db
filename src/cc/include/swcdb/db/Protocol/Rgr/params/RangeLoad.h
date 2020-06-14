
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_rgr_params_RangeLoad_h
#define swc_db_protocol_rgr_params_RangeLoad_h

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


class RangeLoad : public Common::Params::ColRangeId {
  public:

  RangeLoad() {}

  RangeLoad(cid_t cid, rid_t rid, DB::Schema::Ptr schema) 
            : Common::Params::ColRangeId(cid, rid), schema(schema) {
  }
             
  virtual ~RangeLoad() {}

  DB::Schema::Ptr schema;
  
  private:

  size_t encoded_length_internal() const {
    return ColRangeId::encoded_length_internal() + schema->encoded_length();
  }
    
  void encode_internal(uint8_t **bufp) const {
    ColRangeId::encode_internal(bufp);
    schema->encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    ColRangeId::decode_internal(version, bufp, remainp);
    schema = std::make_shared<DB::Schema>(bufp, remainp);
  }

};
  
class RangeLoaded : public Serializable {
  public:
  
  RangeLoaded(const Types::KeySeq key_seq)
              : intval(false), interval(key_seq) { 
  }

  //RangeLoaded(const DB::Cells::Interval& interval): interval(interval) {}

  virtual ~RangeLoaded() {}
  
  bool                intval;
  DB::Cells::Interval interval;

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return 1 + (intval ? interval.encoded_length() : 0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_bool(bufp, intval);
    if(intval)
      interval.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    if(intval = Serialization::decode_bool(bufp, remainp))
      interval.decode(bufp, remainp);
  }

};

}}}}

#endif // swc_db_protocol_rgr_params_RangeLoad_h

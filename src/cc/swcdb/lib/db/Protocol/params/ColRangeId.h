
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_ColRangeId_h
#define swc_db_protocol_params_ColRangeId_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

class ColRangeId : public Serializable {
  public:

  ColRangeId() {}
  ColRangeId(size_t cid, size_t rid) 
            : cid(cid), rid(rid){}
             
  virtual ~ColRangeId() {}

  size_t  cid; 
  size_t  rid;
    
    
  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(rid);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = (size_t)Serialization::decode_vi64(bufp, remainp);
    rid = (size_t)Serialization::decode_vi64(bufp, remainp);
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }

};
  

}}}

#endif // swc_db_protocol_params_ColRangeId_h

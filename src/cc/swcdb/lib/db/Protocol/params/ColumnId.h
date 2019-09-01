
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_ColumnId_h
#define swc_db_protocol_params_ColumnId_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class ColumnId : public Serializable {
  public:

    ColumnId() {}
    ColumnId(size_t cid) : cid(cid){}
             
    virtual ~ColumnId() {}

    size_t  cid; 
    
  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      return Serialization::encoded_length_vi64(cid);
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_vi64(bufp, cid);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      cid = (size_t)Serialization::decode_vi64(bufp, remainp);
    }

  };
  

}}}

#endif // swc_db_protocol_params_ColumnId_h

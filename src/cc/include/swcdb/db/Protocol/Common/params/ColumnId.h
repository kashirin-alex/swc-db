
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_common_params_ColumnId_h
#define swc_db_protocol_common_params_ColumnId_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Common { namespace Params {


  class ColumnId : public Serializable {
  public:

    ColumnId(int64_t cid = 0) 
             : cid(cid){
    }
             
    virtual ~ColumnId() {}

    int64_t  cid; 
    
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
  

}}}}

#endif // swc_db_protocol_common_params_ColumnId_h

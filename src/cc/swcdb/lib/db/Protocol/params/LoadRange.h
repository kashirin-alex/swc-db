
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_LoadRange_h
#define swc_db_protocol_params_LoadRange_h

#include "swcdb/lib/core/Serializable.h"
#include "swcdb/lib/db/Types/RsRole.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class LoadRange : public Serializable {
  public:

    LoadRange() {}
    LoadRange(Types::RsRole role, size_t cid, size_t rid) 
             : cid(cid), rid(rid), role(role){}
             
    virtual ~LoadRange() {}

    size_t  cid; 
    size_t  rid;
    Types::RsRole  role;
    
  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      size_t len = 1;
      if(role == Types::RsRole::MASTER)
        return len;
      return len+Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(rid);
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i8(bufp, (uint8_t)role);
      if(role != Types::RsRole::MASTER) {
        Serialization::encode_vi64(bufp, cid);
        Serialization::encode_vi64(bufp, rid);
      }
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      role = (Types::RsRole)Serialization::decode_i8(bufp, remainp);
      if(role != Types::RsRole::MASTER) {
        cid = (size_t)Serialization::decode_vi64(bufp, remainp);
        rid = (size_t)Serialization::decode_vi64(bufp, remainp);
      }
    }

  };
  

}}}

#endif // swc_db_protocol_params_LoadRange_h

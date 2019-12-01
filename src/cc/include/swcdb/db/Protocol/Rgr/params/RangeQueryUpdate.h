
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_RangeQueryUpdate_h
#define swc_db_protocol_rgr_params_RangeQueryUpdate_h

#include "swcdb/core/Serializable.h"

#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeQueryUpdateReq : public Serializable {
  public:

  RangeQueryUpdateReq() {}

  RangeQueryUpdateReq(int64_t cid, int64_t rid)
                      : cid(cid), rid(rid) {}

  virtual ~RangeQueryUpdateReq(){ }

  int64_t           cid;
  int64_t           rid;
  
  const std::string to_string() {
    std::string s("RangeQueryUpdateReq(");
    s.append(" cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return  Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(rid);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
  }

};



class RangeQueryUpdateRsp  : public Serializable {
  public:

  RangeQueryUpdateRsp(int err = 0) : err(err) {  }

  int32_t   err;

  const std::string to_string() {
    std::string s("RangeQueryUpdateRsp(");
    s.append("err=");
    s.append(std::to_string(err));
    s.append("(");
    s.append(Error::get_text(err));
    s.append("))");
    return s;
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return  Serialization::encoded_length_vi32(err);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    err = Serialization::decode_vi32(bufp, remainp);
  }

};
  

}}}}

#endif // swc_db_protocol_rgr_params_RangeQueryUpdate_h

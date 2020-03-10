
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_RangeCreate_h
#define swc_db_protocol_mngr_params_RangeCreate_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RangeCreateReq : public Serializable {
  public:

  RangeCreateReq(int64_t cid=0, int64_t rgr_id=0) : cid(cid), rgr_id(rgr_id) {}

  virtual ~RangeCreateReq(){ }
  
  int64_t        cid;
  int64_t        rgr_id;
  
  const std::string to_string() const {
    std::string s("RangeCreateReq(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rgr_id=");
    s.append(std::to_string(rgr_id));
    return s;
  }

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rgr_id);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rgr_id);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rgr_id = Serialization::decode_vi64(bufp, remainp);
  }

};



class RangeCreateRsp : public Serializable {
  public:

  RangeCreateRsp(): err(0), rid(0) {}

  virtual ~RangeCreateRsp() {}

  int             err;        
  int64_t         rid; 

  const std::string to_string() const {
    std::string s("RangeCreateRsp(");
    s.append("err=");
    s.append(std::to_string(err));
    if(!err) {
      s.append(" rid=");
      s.append(std::to_string(rid));
    } else {
      s.append("(");
      s.append(Error::get_text(err));
      s.append(")");
    }
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi32(err) 
          + (err ? 0 : Serialization::encoded_length_vi64(rid));
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    if(!err)
      Serialization::encode_vi64(bufp, rid);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    err = Serialization::decode_vi32(bufp, remainp);
    if(!err)
      rid = Serialization::decode_vi64(bufp, remainp);
  }

};
  

}}}}

#endif // swc_db_protocol_mngr_params_RangeCreate_h

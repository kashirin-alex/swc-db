
/*
 * Copyright (C) 2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_ColumnCompact_h
#define swc_db_protocol_rgr_params_ColumnCompact_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


class ColumnCompactReq : public Serializable {
  public:

  ColumnCompactReq(int64_t cid=0) : cid(cid) {}

  virtual ~ColumnCompactReq(){ }
  
  int64_t        cid;
  
  const std::string to_string() const {
    std::string s("ColumnCompactReq(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const  {
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
    cid = Serialization::decode_vi64(bufp, remainp);
  }

};



class ColumnCompactRsp : public Serializable {
  public:

  ColumnCompactRsp(): err(0) {}

  int             err;        

  const std::string to_string() const {
    std::string s("ColumnCompactRsp(");
    s.append("err=");
    s.append(std::to_string(err));
    if(err) {
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
    return Serialization::encoded_length_vi32(err);
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

#endif // swc_db_protocol_rgr_params_ColumnCompact_h

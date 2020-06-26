
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_mngr_params_RangeRemove_h
#define swc_db_protocol_mngr_params_RangeRemove_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RangeRemoveReq : public Serializable {
  public:

  RangeRemoveReq(cid_t cid=0, rid_t rid=0) 
                : cid(cid), rid(rid) {
  }

  virtual ~RangeRemoveReq(){ }
  
  cid_t        cid;
  rid_t        rid;
  
  std::string to_string() const {
    std::string s("RangeRemoveReq(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    return s;
  }

  private:

  size_t internal_encoded_length() const {
    return Serialization::encoded_length_vi64(cid)
         + Serialization::encoded_length_vi64(rid);
  }
    
  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
  }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
  }

};



class RangeRemoveRsp : public Serializable {
  public:

  RangeRemoveRsp(): err(Error::OK) {}

  virtual ~RangeRemoveRsp() {}
  
  int             err;        

  std::string to_string() const {
    std::string s("RangeRemoveRsp(");
    s.append("err=");
    s.append(std::to_string(err));
    s.append("(");
    s.append(Error::get_text(err));
    s.append(")");
    s.append(")");
    return s;
  }

  private:

  size_t internal_encoded_length() const {
    return Serialization::encoded_length_vi32(err);
  }
    
  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_vi32(bufp, err);
  }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    err = Serialization::decode_vi32(bufp, remainp);
  }

};
  

}}}}

#endif // swc_db_protocol_mngr_params_RangeRemove_h

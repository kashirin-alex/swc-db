
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_RangeQuerySelect_h
#define swc_db_protocol_rgr_params_RangeQuerySelect_h

#include "swcdb/lib/core/Serializable.h"

#include "swcdb/lib/db/Cells/Cell.h"
#include "swcdb/lib/db/Cells/SpecsInterval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeQuerySelectReq : public Serializable {
  public:

  RangeQuerySelectReq() {}

  RangeQuerySelectReq(int64_t cid, int64_t rid, 
                      const DB::Specs::Interval& interval)
                      : cid(cid), rid(rid), interval(interval) {}

  virtual ~RangeQuerySelectReq(){ }

  int64_t              cid;
  int64_t              rid;
  DB::Specs::Interval  interval;
  
  const std::string to_string() {
    std::string s("RangeQuerySelectReq(");
    s.append(" cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" ");
    s.append(interval.to_string());
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return  Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(rid)
          + interval.encoded_length();
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    interval.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    interval.decode(bufp, remainp);
  }

};



class RangeQuerySelectRsp  : public Serializable {
  public:

  RangeQuerySelectRsp(int err = 0) : err(err) {  }

  int32_t   err;

  const std::string to_string() {
    std::string s("RangeQuerySelectRsp(");
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

#endif // swc_db_protocol_rgr_params_RangeQuerySelect_h

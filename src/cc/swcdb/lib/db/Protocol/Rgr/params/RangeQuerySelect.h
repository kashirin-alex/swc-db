
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
                      const DB::Specs::Interval& interval, 
                      uint32_t limit_buffer_sz = 0)
                      : cid(cid), rid(rid), interval(interval), 
                        limit_buffer_sz(limit_buffer_sz) {}

  virtual ~RangeQuerySelectReq(){ }

  int64_t              cid;
  int64_t              rid;
  DB::Specs::Interval  interval;
  uint32_t             limit_buffer_sz;
  
  const std::string to_string() {
    std::string s("RangeQuerySelectReq(");
    s.append(" cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" ");
    s.append(interval.to_string());
    s.append(" limit_buffer_sz=");
    s.append(std::to_string(limit_buffer_sz));
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
          + interval.encoded_length()
          + Serialization::encoded_length_vi32(limit_buffer_sz);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    interval.encode(bufp);
    Serialization::encode_vi32(bufp, limit_buffer_sz);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    interval.decode(bufp, remainp);
    limit_buffer_sz = Serialization::decode_vi32(bufp, remainp);
  }

};



class RangeQuerySelectRsp  : public Serializable {
  public:

  RangeQuerySelectRsp(int err = 0, bool reached_limit=false) 
                      : err(err), reached_limit(reached_limit) {  
  }
  
  RangeQuerySelectRsp(StaticBuffer& data)
                      : data(data) {
  }

  int32_t         err;
  bool            reached_limit;
  
  StaticBuffer    data;
  const std::string to_string() const {
    std::string s("RangeQuerySelectRsp(");
    s.append("err=");
    s.append(std::to_string(err));
    s.append("(");
    s.append(Error::get_text(err));
    s.append(")");
    s.append(" reached_limit=");
    s.append(std::to_string(reached_limit));
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return  Serialization::encoded_length_vi32(err)
          + 1;
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    Serialization::encode_bool(bufp, reached_limit);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    err = Serialization::decode_vi32(bufp, remainp);
    reached_limit = Serialization::decode_bool(bufp, remainp);
  }

};
  

}}}}

#endif // swc_db_protocol_rgr_params_RangeQuerySelect_h

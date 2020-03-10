
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


RangeQuerySelectReq::RangeQuerySelectReq() {}

RangeQuerySelectReq::RangeQuerySelectReq(int64_t cid, int64_t rid, 
                                         const DB::Specs::Interval& interval, 
                                         uint32_t limit_buffer_sz)
                                        : cid(cid), rid(rid), 
                                          interval(interval), 
                                          limit_buffer_sz(limit_buffer_sz) {}

RangeQuerySelectReq::~RangeQuerySelectReq() { }

const std::string RangeQuerySelectReq::to_string() const {
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

uint8_t RangeQuerySelectReq::encoding_version() const  {
  return 1; 
}

size_t RangeQuerySelectReq::encoded_length_internal() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + interval.encoded_length()
        + Serialization::encoded_length_vi32(limit_buffer_sz);
}
  
void RangeQuerySelectReq::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  interval.encode(bufp);
  Serialization::encode_vi32(bufp, limit_buffer_sz);
}
  
void RangeQuerySelectReq::decode_internal(uint8_t version, 
                                          const uint8_t **bufp, 
                                          size_t *remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp);
  limit_buffer_sz = Serialization::decode_vi32(bufp, remainp);
}



RangeQuerySelectRsp::RangeQuerySelectRsp(int err, bool reached_limit,
                                         size_t offset)
                                        : err(err), 
                                        reached_limit(reached_limit), 
                                        offset(offset) {  
}

RangeQuerySelectRsp::RangeQuerySelectRsp(StaticBuffer& data)
                    : data(data), err(0), reached_limit(false), offset(0) {
}

RangeQuerySelectRsp::~RangeQuerySelectRsp() { }

const std::string RangeQuerySelectRsp::to_string() const {
  std::string s("RangeQuerySelectRsp(");
  s.append("err=");
  s.append(std::to_string(err));
  s.append("(");
  s.append(Error::get_text(err));
  s.append(")");
  s.append(" reached_limit=");
  s.append(std::to_string(reached_limit));
  s.append(" offset=");
  s.append(std::to_string(offset));
  s.append(" data.size=");
  s.append(std::to_string(data.size));
  s.append(")");
  return s;
}

uint8_t RangeQuerySelectRsp::encoding_version() const {
  return 1;
}
  
size_t RangeQuerySelectRsp::encoded_length_internal() const {
  return Serialization::encoded_length_vi32(err) 
        + 1 
        + Serialization::encoded_length_vi64(offset);
}
  
void RangeQuerySelectRsp::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, err);
  Serialization::encode_bool(bufp, reached_limit);
  Serialization::encode_vi64(bufp, offset);
}
  
void RangeQuerySelectRsp::decode_internal(uint8_t version, 
                                          const uint8_t **bufp,
                                          size_t *remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  reached_limit = Serialization::decode_bool(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
}


}}}}

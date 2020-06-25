
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


RangeQuerySelectReq::RangeQuerySelectReq() {}

RangeQuerySelectReq::RangeQuerySelectReq(cid_t cid, rid_t rid, 
                                         const DB::Specs::Interval& interval)
                                        : cid(cid), rid(rid), 
                                          interval(interval) {
}

RangeQuerySelectReq::~RangeQuerySelectReq() { }

std::string RangeQuerySelectReq::to_string() const {
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

size_t RangeQuerySelectReq::encoded_length_internal() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + interval.encoded_length();
}
  
void RangeQuerySelectReq::encode_internal(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  interval.encode(bufp);
}
  
void RangeQuerySelectReq::decode_internal(const uint8_t** bufp, 
                                          size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp);
}



RangeQuerySelectRsp::RangeQuerySelectRsp(
                int err, bool reached_limit, uint64_t offset)
              : err(err), 
                reached_limit(reached_limit), offset(offset) {  
}

RangeQuerySelectRsp::RangeQuerySelectRsp(StaticBuffer& data)
                    : data(data), 
                      err(0), reached_limit(false), offset(0) {
}

RangeQuerySelectRsp::~RangeQuerySelectRsp() { }

std::string RangeQuerySelectRsp::to_string() const {
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

size_t RangeQuerySelectRsp::encoded_length_internal() const {
  return Serialization::encoded_length_vi32(err) 
        + 1 
        + Serialization::encoded_length_vi64(offset);
}
  
void RangeQuerySelectRsp::encode_internal(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  Serialization::encode_bool(bufp, reached_limit);
  Serialization::encode_vi64(bufp, offset);
}
  
void RangeQuerySelectRsp::decode_internal(const uint8_t** bufp,
                                          size_t* remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  reached_limit = Serialization::decode_bool(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
}


}}}}

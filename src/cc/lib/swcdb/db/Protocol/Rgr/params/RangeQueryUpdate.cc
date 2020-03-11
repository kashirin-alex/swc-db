
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 


#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Error.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

RangeQueryUpdateReq::RangeQueryUpdateReq() {}

RangeQueryUpdateReq::RangeQueryUpdateReq(int64_t cid, int64_t rid)
                                        : cid(cid), rid(rid) {}

RangeQueryUpdateReq::~RangeQueryUpdateReq() { }

const std::string RangeQueryUpdateReq::to_string() const {
  std::string s("RangeQueryUpdateReq(");
  s.append(" cid=");
  s.append(std::to_string(cid));
  s.append(" rid=");
  s.append(std::to_string(rid));
  s.append(")");
  return s;
}

uint8_t RangeQueryUpdateReq::encoding_version() const  {
  return 1; 
}

size_t RangeQueryUpdateReq::encoded_length_internal() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid);
}
  
void RangeQueryUpdateReq::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
}
  
void RangeQueryUpdateReq::decode_internal(uint8_t version, const uint8_t **bufp, 
                     size_t *remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
}




RangeQueryUpdateRsp::RangeQueryUpdateRsp(int err) : err(err) {  }

RangeQueryUpdateRsp::RangeQueryUpdateRsp(
                      int err, const DB::Cell::Key& range_end) 
                    : err(err), range_end(range_end) {  
}

RangeQueryUpdateRsp::~RangeQueryUpdateRsp() { }

const std::string RangeQueryUpdateRsp::to_string() const {
  std::string s("RangeQueryUpdateRsp(");
  s.append("err=");
  s.append(std::to_string(err));
  s.append("(");
  s.append(Error::get_text(err));
  s.append(")");
  if(err == Error::RANGE_END_EARLIER) {
    s.append(" range_end=");
    s.append(range_end.to_string());
  }
  s.append(")");
  return s;
}

uint8_t RangeQueryUpdateRsp::encoding_version() const {
  return 1;
}
  
size_t RangeQueryUpdateRsp::encoded_length_internal() const {
  return  Serialization::encoded_length_vi32(err)
        + (err == Error::RANGE_END_EARLIER ? range_end.encoded_length() : 0);
}
  
void RangeQueryUpdateRsp::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(err == Error::RANGE_END_EARLIER) 
    range_end.encode(bufp);
}
  
void RangeQueryUpdateRsp::decode_internal(uint8_t version, const uint8_t **bufp, 
                     size_t *remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  if(err == Error::RANGE_END_EARLIER) 
    range_end.decode(bufp, remainp, true);
}



}}}}

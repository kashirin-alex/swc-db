
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


RangeLocateReq::RangeLocateReq(cid_t cid, rid_t rid)
                              : cid(cid), rid(rid), flags(0) {}

RangeLocateReq::~RangeLocateReq() { }

std::string RangeLocateReq::to_string() const {
  std::string s("RangeLocateReq(");
  s.append("cid=");
  s.append(std::to_string(cid));
  s.append(" rid=");
  s.append(std::to_string(rid));

  s.append(" flags=");
  s.append(std::to_string((int)flags));
  s.append(" RangeBegin");
  s.append(range_begin.to_string());
  s.append(" RangeEnd");
  s.append(range_end.to_string());
  if(flags & NEXT_RANGE) {
    s.append(" RangeOffset");
    s.append(range_offset.to_string());
  }
  s.append(")");
  return s;
}

size_t RangeLocateReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + range_begin.encoded_length() 
        + range_end.encoded_length()
        + 1
          + (flags & NEXT_RANGE ? range_offset.encoded_length() : 0);
}
  
void RangeLocateReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  range_begin.encode(bufp);
  range_end.encode(bufp);
  Serialization::encode_i8(bufp, flags);
  if(flags & NEXT_RANGE)
    range_offset.encode(bufp);
}
  
void RangeLocateReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  range_begin.decode(bufp, remainp);
  range_end.decode(bufp, remainp);
  flags = Serialization::decode_i8(bufp, remainp);
  if(flags & NEXT_RANGE)
    range_offset.decode(bufp, remainp);
}



RangeLocateRsp::RangeLocateRsp(int err) 
                              : err(err), cid(0), rid(0) { }

RangeLocateRsp::~RangeLocateRsp() { }

std::string RangeLocateRsp::to_string() const {
  std::string s("Range(");
  s.append("err=");
  s.append(std::to_string(err));
  if(err) {
    s.append("(");
    s.append(Error::get_text(err));
    s.append(")");
  } else {
    s.append(" cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));

    s.append(" RangeBegin");
    s.append(range_begin.to_string());
    s.append(" RangeEnd");
    s.append(range_end.to_string());
  }
  s.append(")");
  return s;
}

size_t RangeLocateRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(err) 
    + (err ? 0 : (
        Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rid)
      + range_end.encoded_length()
      + range_begin.encoded_length()
      ) );
}
  
void RangeLocateRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(!err) {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    range_end.encode(bufp);
    range_begin.encode(bufp);
  }
}
  
void RangeLocateRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  if(!(err = Serialization::decode_vi32(bufp, remainp))) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    range_end.decode(bufp, remainp, true);
    range_begin.decode(bufp, remainp, true);
  }
}



}}}}


/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"
#include "swcdb/core/Serialization.h"



namespace SWC { namespace Protocol { namespace Mngr { namespace Params {

RgrGetReq::RgrGetReq(int64_t cid, int64_t rid, bool next_range) 
                    : cid(cid), rid(rid), next_range(next_range) {
}

RgrGetReq::~RgrGetReq() { }

std::string RgrGetReq::to_string() {
  std::string s("Ranger(");
  s.append("cid=");
  s.append(std::to_string(cid));
  s.append(" rid=");
  s.append(std::to_string(rid));
  if(!rid) {
    s.append(" next_range=");
    s.append(std::to_string(next_range));
    s.append(" RangeBegin");
    s.append(range_begin.to_string());
    s.append(" RangeEnd");
    s.append(range_end.to_string());
  }
  s.append(")");
  return s;
}

uint8_t RgrGetReq::encoding_version() const  {
  return 1; 
}

size_t RgrGetReq::encoded_length_internal() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + (rid ? 0 :
           (range_begin.encoded_length() 
            + range_end.encoded_length() 
            + 1)
          );
}
  
void RgrGetReq::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  if(!rid) {
    range_begin.encode(bufp);
    range_end.encode(bufp);
    Serialization::encode_bool(bufp, next_range);
  }
}
  
void RgrGetReq::decode_internal(uint8_t version, const uint8_t **bufp, 
                                size_t *remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  if(!rid) {
    range_begin.decode(bufp, remainp);
    range_end.decode(bufp, remainp);
    next_range = Serialization::decode_bool(bufp, remainp);
  }
}



RgrGetRsp::RgrGetRsp(int64_t cid, int64_t rid)
                    : err(0), cid(cid), rid(rid) { }

RgrGetRsp::~RgrGetRsp() { }

std::string RgrGetRsp::to_string() const {
  std::string s("Ranger(");
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
    s.append(" ");
    s.append(Common::Params::HostEndPoints::to_string());
    if(cid == 1) {
      s.append(" RangeBegin");
      s.append(range_begin.to_string());
      s.append(" RangeEnd");
      s.append(range_end.to_string());
    }
  }
  s.append(")");
  return s;
}

uint8_t RgrGetRsp::encoding_version() const {
  return 1;
}
  
size_t RgrGetRsp::encoded_length_internal() const {
  return  Serialization::encoded_length_vi32(err) 
  + (err ? 0 :
      (Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rid)
      + Common::Params::HostEndPoints::encoded_length_internal()
      + (cid == 1 
        ? (range_end.encoded_length() + range_begin.encoded_length())
        : 0)
      )
    );
}
  
void RgrGetRsp::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(!err) {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    Common::Params::HostEndPoints::encode_internal(bufp);
    if(cid == 1) {
      range_end.encode(bufp);
      range_begin.encode(bufp);
    }
  }
}
  
void RgrGetRsp::decode_internal(uint8_t version, const uint8_t **bufp, 
                                size_t *remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  if(!err) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    Common::Params::HostEndPoints::decode_internal(version, bufp, remainp);
    if(cid == 1) {
      range_end.decode(bufp, remainp, true);
      range_begin.decode(bufp, remainp, true);
    }
  }
}


}}}}

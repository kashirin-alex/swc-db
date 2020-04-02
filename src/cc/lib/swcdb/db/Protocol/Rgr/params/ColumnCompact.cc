
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


ColumnCompactReq::ColumnCompactReq(int64_t cid) : cid(cid) {}

ColumnCompactReq::~ColumnCompactReq() { }

std::string ColumnCompactReq::to_string() const {
  std::string s("ColumnCompactReq(");
  s.append("cid=");
  s.append(std::to_string(cid));
  s.append(")");
  return s;
}

uint8_t ColumnCompactReq::encoding_version() const  {
  return 1; 
}

size_t ColumnCompactReq::encoded_length_internal() const {
  return Serialization::encoded_length_vi64(cid);
}
  
void ColumnCompactReq::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, cid);
}
  
void ColumnCompactReq::decode_internal(uint8_t version, const uint8_t **bufp,
                                       size_t *remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
}



ColumnCompactRsp::ColumnCompactRsp(): err(0) {}

ColumnCompactRsp::~ColumnCompactRsp() { }

std::string ColumnCompactRsp::to_string() const {
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

uint8_t ColumnCompactRsp::encoding_version() const {
  return 1;
}
  
size_t ColumnCompactRsp::encoded_length_internal() const {
  return Serialization::encoded_length_vi32(err);
}
  
void ColumnCompactRsp::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, err);
}
  
void ColumnCompactRsp::decode_internal(uint8_t version, const uint8_t **bufp, 
                                       size_t *remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
}



}}}}
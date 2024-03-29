/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


std::string ColumnCompactReq::to_string() const {
  std::string s("ColumnCompactReq(");
  s.append("cid=");
  s.append(std::to_string(cid));
  s.append(")");
  return s;
}

size_t ColumnCompactReq::internal_encoded_length() const {
  return Serialization::encoded_length_vi64(cid);
}

void ColumnCompactReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
}

void ColumnCompactReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
}




ColumnCompactRsp::ColumnCompactRsp(int a_err,
                                   const uint8_t* ptr, size_t remain)
                                   noexcept : err(a_err) {
  if(!err) try {
    decode(&ptr, &remain);
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
  }
}

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

size_t ColumnCompactRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(err);
}

void ColumnCompactRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
}

void ColumnCompactRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
}


}}}}}


/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


MngrActiveReq::MngrActiveReq(uint8_t role, cid_t cid) 
                            : role(role), cid(cid) {
}

MngrActiveReq::~MngrActiveReq() { }

size_t MngrActiveReq::internal_encoded_length() const {
  return 1 + Serialization::encoded_length_vi64(cid);
}

void MngrActiveReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, role);
  Serialization::encode_vi64(bufp, cid);
}

void MngrActiveReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  role = Serialization::decode_i8(bufp, remainp);
  cid = Serialization::decode_vi64(bufp, remainp);
}



MngrActiveRsp::MngrActiveRsp() {}

MngrActiveRsp::MngrActiveRsp(const Comm::EndPoints& endpoints) 
                            : Common::Params::HostEndPoints(endpoints), 
                              available(endpoints.size()>0) { 
}

MngrActiveRsp::~MngrActiveRsp() { }

size_t MngrActiveRsp::internal_encoded_length() const {
  size_t len = 1;
  if(available)
    len += Common::Params::HostEndPoints::internal_encoded_length();
  return len;
}

void MngrActiveRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_bool(bufp, available);
  if(available) {
    Common::Params::HostEndPoints::internal_encode(bufp);
  }
}

void MngrActiveRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  available = Serialization::decode_bool(bufp, remainp);
  if(available) {
    Common::Params::HostEndPoints::internal_decode(bufp, remainp);
  }
}



}}}}

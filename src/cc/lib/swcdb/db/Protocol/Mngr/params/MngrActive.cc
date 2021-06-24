/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {



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



size_t MngrActiveRsp::internal_encoded_length() const {
  return Serialization::encoded_length(endpoints);
}

void MngrActiveRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode(bufp, endpoints);
}

void MngrActiveRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  Serialization::decode(bufp, remainp, endpoints);
}



}}}}}

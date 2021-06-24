/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/Protocol/Mngr/params/MngrState.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {



size_t MngrState::internal_encoded_length() const {
  size_t len = 12 + Serialization::encoded_length(mngr_host);
  for(auto& h : states)
    len += h->encoded_length();
  return len;
}

void MngrState::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i32(bufp, states.size());
  Serialization::encode_i64(bufp, token);
  Serialization::encode(bufp, mngr_host);
  for(auto& h : states)
    h->encode(bufp);
}

void MngrState::internal_decode(const uint8_t** bufp, size_t* remainp) {
  size_t len = Serialization::decode_i32(bufp, remainp);
  token = Serialization::decode_i64(bufp, remainp);
  mngr_host = Serialization::decode(bufp, remainp);
  states.clear();
  states.reserve(len);
  for(size_t i=0; i<len; ++i)
    states.emplace_back(new Manager::MngrStatus(bufp, remainp));
}


}}}}}

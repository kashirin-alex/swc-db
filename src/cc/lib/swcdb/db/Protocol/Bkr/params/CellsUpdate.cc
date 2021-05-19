/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Params {



void CellsUpdateReq::print(std::ostream& out) const {
  out << "CellsUpdateReq(cid=" << cid << ')';
}

size_t CellsUpdateReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid);
}

void CellsUpdateReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
}

void CellsUpdateReq::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
}




void CellsUpdateRsp::print(std::ostream& out) const {
  Error::print(out << "CellsUpdateRsp(", err);
  out << ')';
}

size_t CellsUpdateRsp::internal_encoded_length() const {
  return  Serialization::encoded_length_vi32(err);
}

void CellsUpdateRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
}

void CellsUpdateRsp::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
}



}}}}}

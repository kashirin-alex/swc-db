/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/params/CellsSelect.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Params {



void CellsSelectReq::print(std::ostream& out) const {
  specs.print(out << "CellsSelectReq(");
  out << ')';
}

size_t CellsSelectReq::internal_encoded_length() const {
  return specs.encoded_length();
}

void CellsSelectReq::internal_encode(uint8_t** bufp) const {
  specs.encode(bufp);
}

void CellsSelectReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  specs.decode(bufp, remainp);
}




void CellsSelectRsp::print(std::ostream& out) const {
  Error::print(out << "CellsSelectRsp(", err);
  if(!err)
    out << " cid=" << cid << " more=" << more;
  out << ')';
}

size_t CellsSelectRsp::internal_encoded_length() const {
  return  Serialization::encoded_length_vi32(err) +
          (err ? 0 : (Serialization::encoded_length_vi64(cid) + 1));
}

void CellsSelectRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(!err) {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_bool(bufp, more);
  }
}

void CellsSelectRsp::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  if(!(err = Serialization::decode_vi32(bufp, remainp))) {
    cid = Serialization::decode_vi64(bufp, remainp);
    more = Serialization::decode_bool(bufp, remainp);
  }
}



}}}}}

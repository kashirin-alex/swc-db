/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/params/CellsSelect.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Params {



CellsSelectReq::CellsSelectReq(cid_t cid,
                               const DB::Specs::Interval& interval)
                              : cid(cid), interval(interval) {
}

void CellsSelectReq::print(std::ostream& out) const {
  out << "CellsSelectReq(cid=" << cid;
  interval.print(out << ' ');
  out << ')';
}

size_t CellsSelectReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid)
        + interval.encoded_length();
}

void CellsSelectReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  interval.encode(bufp);
}

void CellsSelectReq::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp, true);
}



void CellsSelectRsp::print(std::ostream& out) const {
  Error::print(out << "CellsSelectRsp(", err);
  out << " more=" << more
      << " offset=" << offset
      << " data.size=" << data.size
      << ')';
}

size_t CellsSelectRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(err)
        + 1
        + Serialization::encoded_length_vi64(offset);
}

void CellsSelectRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  Serialization::encode_bool(bufp, more);
  Serialization::encode_vi64(bufp, offset);
}

void CellsSelectRsp::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  more = Serialization::decode_bool(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
}



}}}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


RangeQuerySelectReq::RangeQuerySelectReq() {}

RangeQuerySelectReq::RangeQuerySelectReq(cid_t cid, rid_t rid,
                                         const DB::Specs::Interval& interval)
                                        : cid(cid), rid(rid),
                                          interval(interval) {
}

RangeQuerySelectReq::~RangeQuerySelectReq() { }

void RangeQuerySelectReq::print(std::ostream& out) const {
  out << "RangeQuerySelectReq(cid=" << cid << " rid=" << rid;
  interval.print(out << ' ');
  out << ')';
}

size_t RangeQuerySelectReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + interval.encoded_length();
}

void RangeQuerySelectReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  interval.encode(bufp);
}

void RangeQuerySelectReq::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp);
}



RangeQuerySelectRsp::RangeQuerySelectRsp(
                int err, bool reached_limit, uint64_t offset)
              : err(err), reached_limit(reached_limit), offset(offset) {
}

RangeQuerySelectRsp::RangeQuerySelectRsp(int err, StaticBuffer& data)
              : err(err), reached_limit(false), offset(0), data(data) {
}

RangeQuerySelectRsp::~RangeQuerySelectRsp() { }

void RangeQuerySelectRsp::print(std::ostream& out) const {
  Error::print(out << "RangeQuerySelectRsp(", err);
  out << " reached_limit=" << reached_limit
      << " offset=" << offset
      << " data.size=" << data.size
      << ')';
}

size_t RangeQuerySelectRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(err)
        + 1
        + Serialization::encoded_length_vi64(offset);
}

void RangeQuerySelectRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  Serialization::encode_bool(bufp, reached_limit);
  Serialization::encode_vi64(bufp, offset);
}

void RangeQuerySelectRsp::internal_decode(const uint8_t** bufp,
                                          size_t* remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  reached_limit = Serialization::decode_bool(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
}


}}}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


RangeLocateReq::RangeLocateReq(cid_t cid, rid_t rid)
                              : cid(cid), rid(rid), flags(0) {}

RangeLocateReq::~RangeLocateReq() { }

void RangeLocateReq::print(std::ostream& out) const {
  out << "RangeLocateReq(cid=" << cid << " rid=" << rid
      << " flags=" << int(flags);
  range_begin.print(out << " RangeBegin");
  range_end.print(out << " RangeEnd");
  if(flags & NEXT_RANGE)
    range_offset.print(out << " RangeOffset");
  out << ')';
}

size_t RangeLocateReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + range_begin.encoded_length()
        + range_end.encoded_length()
        + 1
          + (flags & NEXT_RANGE ? range_offset.encoded_length() : 0);
}

void RangeLocateReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  range_begin.encode(bufp);
  range_end.encode(bufp);
  Serialization::encode_i8(bufp, flags);
  if(flags & NEXT_RANGE)
    range_offset.encode(bufp);
}

void RangeLocateReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  range_begin.decode(bufp, remainp, false);
  range_end.decode(bufp, remainp, false);
  flags = Serialization::decode_i8(bufp, remainp);
  if(flags & NEXT_RANGE)
    range_offset.decode(bufp, remainp, false);
}



RangeLocateRsp::RangeLocateRsp(int err)
                              : err(err), cid(0), rid(0) { }

RangeLocateRsp::~RangeLocateRsp() { }

void RangeLocateRsp::print(std::ostream& out) const {
  out << "RangeLocated(";
  Error::print(out, err);
  if(!err) {
    out << " cid=" << cid << " rid=" << rid;
    range_begin.print(out << " RangeBegin");
    range_end.print(out << " RangeEnd");
  }
  out << ')';
}

size_t RangeLocateRsp::internal_encoded_length() const {
  return Serialization::encoded_length_vi32(err)
    + (err ? 0 : (
        Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rid)
      + range_end.encoded_length()
      + range_begin.encoded_length()
      ) );
}

void RangeLocateRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(!err) {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    range_end.encode(bufp);
    range_begin.encode(bufp);
  }
}

void RangeLocateRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  if(!(err = Serialization::decode_vi32(bufp, remainp))) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    range_end.decode(bufp, remainp, true);
    range_begin.decode(bufp, remainp, true);
  }
}



}}}}}

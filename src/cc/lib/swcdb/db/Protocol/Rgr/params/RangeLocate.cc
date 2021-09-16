/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {



void RangeLocateReq::print(std::ostream& out) const {
  out << "RangeLocateReq(cid=" << cid << " rid=" << rid
      << " flags=" << int(flags);
  range_begin.print(out << " RangeBegin");
  range_end.print(out << " RangeEnd");
  if(flags & NEXT_RANGE || flags & CURRENT_RANGE)
    range_offset.print(out << " RangeOffset");
  if(flags & HAVE_REVISION)
    out << " revision=" << revision;
  out << ')';
}

size_t RangeLocateReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + range_begin.encoded_length()
        + range_end.encoded_length()
        + 1
        + (flags & NEXT_RANGE || flags & CURRENT_RANGE
            ? range_offset.encoded_length() : 0)
        + (flags & HAVE_REVISION
            ? Serialization::encoded_length_vi64(revision) : 0);
}

void RangeLocateReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  range_begin.encode(bufp);
  range_end.encode(bufp);
  Serialization::encode_i8(bufp, flags);
  if(flags & NEXT_RANGE || flags & CURRENT_RANGE)
    range_offset.encode(bufp);
  if(flags & HAVE_REVISION)
    Serialization::encode_vi64(bufp, revision);
}

void RangeLocateReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  range_begin.decode(bufp, remainp, false);
  range_end.decode(bufp, remainp, false);
  flags = Serialization::decode_i8(bufp, remainp);
  if(flags & NEXT_RANGE || flags & CURRENT_RANGE)
    range_offset.decode(bufp, remainp, false);
  if(flags & HAVE_REVISION)
    revision = Serialization::decode_vi64(bufp, remainp);
}




RangeLocateRsp::RangeLocateRsp(int a_err,
                               const uint8_t *ptr, size_t remain) noexcept
                              : err(a_err), cid(0), rid(0) {
  if(!err) try {
    decode(&ptr, &remain);
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
  }
}

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
    range_end.decode(bufp, remainp, false);
    range_begin.decode(bufp, remainp, false);
  }
}



}}}}}

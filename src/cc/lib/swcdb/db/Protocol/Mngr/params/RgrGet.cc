/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/db/Types/SystemColumn.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {



void RgrGetReq::print(std::ostream& out) const {
  out << "Ranger(cid=" << cid << " rid=" << rid;
  if(!rid) {
    out << " next_range=" << next_range;
    range_begin.print(out << " RangeBegin");
    range_end.print(out << " RangeEnd");
  }
  out << ')';
}

size_t RgrGetReq::internal_encoded_length() const {
  return  Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + (rid ? 0 :
           (range_begin.encoded_length()
            + range_end.encoded_length()
            + 1)
          );
}

void RgrGetReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi64(bufp, rid);
  if(!rid) {
    range_begin.encode(bufp);
    range_end.encode(bufp);
    Serialization::encode_bool(bufp, next_range);
  }
}

void RgrGetReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  if(!rid) {
    range_begin.decode(bufp, remainp, false);
    range_end.decode(bufp, remainp, false);
    next_range = Serialization::decode_bool(bufp, remainp);
  }
}


RgrGetRsp::RgrGetRsp(int a_err, const uint8_t* ptr, size_t remain) noexcept
                    : err(a_err) {
  if(!err) try {
    decode(&ptr, &remain);
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
  }
}

void RgrGetRsp::print(std::ostream& out) const {
  out << "Ranger(";
  Error::print(out, err);
  if(!err) {
    out << " cid=" << cid << " rid=" << rid;
    Comm::print(out << ' ', endpoints);
    if(DB::Types::SystemColumn::is_master(cid)) {
      range_begin.print(out << " RangeBegin");
      range_end.print(out << " RangeEnd");
      out << " revision=" << revision;
    }
  }
  out << ')';
}

size_t RgrGetRsp::internal_encoded_length() const {
  return  Serialization::encoded_length_vi32(err)
  + (err ? 0 :
      (Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rid)
      + Serialization::encoded_length(endpoints)
      + (DB::Types::SystemColumn::is_master(cid)
        ? (range_begin.encoded_length() +
           range_end.encoded_length() +
           Serialization::encoded_length_vi64(revision))
        : 0)
      )
    );
}

void RgrGetRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(!err) {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    Serialization::encode(bufp, endpoints);
    if(DB::Types::SystemColumn::is_master(cid)) {
      range_begin.encode(bufp);
      range_end.encode(bufp);
      Serialization::encode_vi64(bufp, revision);
    }
  }
}

void RgrGetRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  err = Serialization::decode_vi32(bufp, remainp);
  if(!err) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    Serialization::decode(bufp, remainp, endpoints);
    if(DB::Types::SystemColumn::is_master(cid)) {
      range_begin.decode(bufp, remainp, false);
      range_end.decode(bufp, remainp, false);
      revision = Serialization::decode_vi64(bufp, remainp);
    }
  }
}


}}}}}


/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Rgr/params/Report.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {
namespace Report {



ReqColumn::ReqColumn(cid_t cid) : cid(cid) { }

ReqColumn::~ReqColumn(){ }

size_t ReqColumn::internal_encoded_length() const {
  return Serialization::encoded_length_vi64(cid);
}

void ReqColumn::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
}

void ReqColumn::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
}



RspRes::RspRes() { }

RspRes::~RspRes() { }

void RspRes::display(std::ostream& out, const std::string& offset) const {
  out << offset
      << "Ranger(mem="
      << mem << "MB cpu="
      << cpu << "Mhz ranges="
      << ranges << ")" << std::endl;
}

size_t RspRes::internal_encoded_length() const {
  return  Serialization::encoded_length_vi32(mem)
        + Serialization::encoded_length_vi32(cpu)
        + Serialization::encoded_length_vi64(ranges);
}

void RspRes::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, mem);
  Serialization::encode_vi32(bufp, cpu);
  Serialization::encode_vi64(bufp, ranges);
}

void RspRes::internal_decode(const uint8_t** bufp, size_t* remainp) {
  mem = Serialization::decode_vi32(bufp, remainp);
  cpu = Serialization::decode_vi32(bufp, remainp);
  ranges = Serialization::decode_vi64(bufp, remainp);
}



RspCids::RspCids() { }

RspCids::~RspCids() { }

void RspCids::display(std::ostream& out, const std::string& offset) const {
  std::sort(cids.begin(), cids.end());
  out << offset << "cids=[";
  for(auto& cid : cids)
    out << cid << ',';
  out << ']' << std::endl;
}

size_t RspCids::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi64(cids.size());
  for(auto& cid : cids)
    sz += Serialization::encoded_length_vi64(cid);
  return sz;
}

void RspCids::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cids.size());
  for(auto& cid : cids)
    Serialization::encode_vi64(bufp, cid);
}

void RspCids::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cids.resize(Serialization::decode_vi64(bufp, remainp));
  for(auto it = cids.begin(); it < cids.end(); ++it)
    *it = Serialization::decode_vi64(bufp, remainp);
}



RspColumnRids::RspColumnRids() { }

RspColumnRids::~RspColumnRids() { }

void RspColumnRids::display(std::ostream& out, const std::string& offset) const {
  std::sort(rids.begin(), rids.end());
  out << offset << "rids=[";
  for(auto& rid : rids)
    out << rid << ',';
  out << ']' << std::endl;
}

size_t RspColumnRids::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi64(rids.size());
  for(auto& rid : rids)
    sz += Serialization::encoded_length_vi64(rid);
  return sz;
}

void RspColumnRids::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, rids.size());
  for(auto& rid : rids)
    Serialization::encode_vi64(bufp, rid);
}

void RspColumnRids::internal_decode(const uint8_t** bufp, size_t* remainp) {
  rids.resize(Serialization::decode_vi64(bufp, remainp));
  for(auto it = rids.begin(); it < rids.end(); ++it)
    *it = Serialization::decode_vi64(bufp, remainp);
}



RspColumnsRanges::Range::Range(DB::Types::KeySeq seq)
                              : interval(seq) {
}

RspColumnsRanges::Range::~Range() { }

bool RspColumnsRanges::Range::before(RspColumnsRanges::Range* r1,
                                     RspColumnsRanges::Range* r2) {
  return r2->interval.is_in_end(r1->interval.key_end);
}

size_t RspColumnsRanges::Range::encoded_length () const {
  return Serialization::encoded_length_vi64(rid)
          + interval.encoded_length();
}

void RspColumnsRanges::Range::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, rid);
  interval.encode(bufp);
}

void RspColumnsRanges::Range::decode(const uint8_t** bufp, size_t* remainp) {
  rid = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp, false);
}

void RspColumnsRanges::Range::display(std::ostream& out, bool pretty,
                                      const std::string& offset) const {
  out << offset << "-------------------------------------" << std::endl;
  out << offset << "rid(" << rid << "):" << std::endl;
  out << offset << " begin";
  interval.key_begin.display(out, pretty);
  out << std::endl;
  out << offset << "   end";
  interval.key_end.display(out, pretty);
  out << std::endl;
  out << offset << " " << interval.ts_earliest.value
      << "<=TS<=" << interval.ts_latest.value ;
  out << std::endl;
}


bool RspColumnsRanges::Column::before(RspColumnsRanges::Column* c1,
                                      RspColumnsRanges::Column* c2) {
  return c1->cid < c2->cid;
}

RspColumnsRanges::Column::~Column() {
  for(auto r : ranges)
    delete r;
  ranges.clear();
}

size_t RspColumnsRanges::Column::encoded_length () const {
  size_t sz = Serialization::encoded_length_vi64(cid) + 1
            + Serialization::encoded_length_vi64(mem_bytes)
            + Serialization::encoded_length_vi64(ranges.size());
  for(auto r : ranges)
    sz += r->encoded_length();
  return sz;
}

void RspColumnsRanges::Column::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_i8(bufp, uint8_t(col_seq));
  Serialization::encode_vi64(bufp, mem_bytes);
  Serialization::encode_vi64(bufp, ranges.size());
  for(auto r : ranges)
    r->encode(bufp);
}

void RspColumnsRanges::Column::decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  col_seq = DB::Types::KeySeq(Serialization::decode_i8(bufp, remainp));
  mem_bytes = Serialization::decode_vi64(bufp, remainp);
  for(int64_t n = Serialization::decode_vi64(bufp, remainp); n; --n) {
    auto r = new Range(col_seq);
    r->decode(bufp, remainp);
    ranges.push_back(r);
  }
  std::sort(ranges.begin(), ranges.end(), Range::before);
}

void RspColumnsRanges::Column::display(std::ostream& out, bool pretty,
                                       const std::string& offset) const {
  out << offset << "**************************************" << std::endl;
  out << offset << "cid(" << cid << ") seq("
      << DB::Types::to_string(col_seq) << ") in-memory("
      << mem_bytes << " bytes) ranges("
      << ranges.size() << ')';
  if(!ranges.empty())
    out << ':';
  out << std::endl;
  for(auto& r : ranges)
    r->display(out, pretty, offset+" ");
}


RspColumnsRanges::RspColumnsRanges() : rgrid(0) { }

RspColumnsRanges::RspColumnsRanges(rgrid_t rgrid,
                                   const EndPoints& endpoints)
                                  : rgrid(rgrid), endpoints(endpoints) {
}

RspColumnsRanges::~RspColumnsRanges() {
  for(auto c : columns)
    delete c;
  columns.clear();
}

void RspColumnsRanges::display(std::ostream& out, bool pretty,
                               const std::string& offset) const {
  out << offset << "Ranger: rgrid("<< rgrid << ")"
      << " endpoints(";
  for(auto& endpoint : endpoints)
    out << endpoint << ", ";
  out << ")" << std::endl;

  out << offset << "Columns(" << columns.size() << "):" << std::endl;
  for(auto& c : columns)
    c->display(out, pretty, offset+" ");
}

size_t RspColumnsRanges::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi64(rgrid);
  sz += Serialization::encoded_length_vi32(endpoints.size());
  for(auto& endpoint : endpoints)
    sz += Serialization::encoded_length(endpoint);

  sz += Serialization::encoded_length_vi64(columns.size());
  for(auto c : columns)
    sz += c->encoded_length();
  return sz;
}

void RspColumnsRanges::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, rgrid);
  Serialization::encode_vi32(bufp, endpoints.size());
  for(auto& endpoint : endpoints)
    Serialization::encode(endpoint, bufp);

  Serialization::encode_vi64(bufp, columns.size());
  for(auto c : columns)
    c->encode(bufp);
}

void RspColumnsRanges::internal_decode(const uint8_t** bufp,
                                       size_t* remainp) {
  rgrid = Serialization::decode_vi64(bufp, remainp);

  size_t len = Serialization::decode_vi32(bufp, remainp);
  endpoints.clear();
  endpoints.resize(len);
  for(size_t i=0;i<len;++i)
    endpoints[i] = Serialization::decode(bufp, remainp);

  for(int64_t n = Serialization::decode_vi64(bufp, remainp); n ; --n) {
    auto c = new Column();
    c->decode(bufp, remainp);
    columns.push_back(c);
  }
  std::sort(columns.begin(), columns.end(), Column::before);
}





}
}}}}}

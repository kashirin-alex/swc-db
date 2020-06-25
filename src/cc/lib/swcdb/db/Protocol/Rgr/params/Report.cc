
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */



#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


ReportReq::ReportReq(uint8_t flags): flags(flags) {}

ReportReq::~ReportReq(){ }

size_t ReportReq::internal_encoded_length() const {
  return 1;
}
  
void ReportReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, flags);
}
  
void ReportReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flags = Serialization::decode_i8(bufp, remainp);
}





ReportRsp::Range::Range(Types::KeySeq seq) 
                        : interval(seq) { 
}

ReportRsp::Range::~Range() { }

bool ReportRsp::Range::before(ReportRsp::Range* r1, ReportRsp::Range* r2) {
  return r2->interval.is_in_end(r1->interval.key_end);
}

size_t ReportRsp::Range::encoded_length () const {
  return Serialization::encoded_length_vi64(rid)
          + interval.encoded_length();
} 

void ReportRsp::Range::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, rid);
  interval.encode(bufp);
}

void ReportRsp::Range::decode(const uint8_t** bufp, size_t* remainp) {
  rid = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp);
}

void ReportRsp::Range::display(std::ostream& out, bool pretty, 
                               std::string offset) const {
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



bool ReportRsp::Column::before(ReportRsp::Column* c1, ReportRsp::Column* c2) {
  return c1->cid < c2->cid;
}

ReportRsp::Column::~Column() {
  for(auto r : ranges)
    delete r;
  ranges.clear();
}

size_t ReportRsp::Column::encoded_length () const {
  size_t sz = Serialization::encoded_length_vi64(cid) + 1
            + Serialization::encoded_length_vi64(ranges.size());
  for(auto r : ranges)
    sz += r->encoded_length();
  return sz;
}

void ReportRsp::Column::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_i8(bufp, (uint8_t)col_seq);
  Serialization::encode_vi64(bufp, ranges.size());
  for(auto r : ranges)
    r->encode(bufp);
}

void ReportRsp::Column::decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  col_seq = (Types::KeySeq)Serialization::decode_i8(bufp, remainp);
  for(int64_t n = Serialization::decode_vi64(bufp, remainp); n; --n) {
    auto r = new Range(col_seq);
    r->decode(bufp, remainp);
    ranges.push_back(r);
  }
  std::sort(ranges.begin(), ranges.end(), Range::before); 
}
  
void ReportRsp::Column::display(std::ostream& out, bool pretty, 
                                std::string offset) const {  
  out << offset << "**************************************" << std::endl;
  out << offset << "cid(" << cid << ") seq(" 
      << Types::to_string(col_seq) << ")" 
      << " ranges(" << ranges.size() << "):" << std::endl;
  for(auto& r : ranges)
    r->display(out, pretty, offset+" ");
}




ReportRsp::ReportRsp(int err) : err(err), rgrid(0) { }

ReportRsp::~ReportRsp() {
  for(auto c : columns)
    delete c;
  columns.clear();
}

void ReportRsp::display(std::ostream& out, bool pretty, 
                        std::string offset) const {
  out << offset << "Ranger: rgrid("<< rgrid << ")" 
      << " endpoints(";
  for(auto& endpoint : endpoints)
    out << endpoint << ", ";
  out << ")" << std::endl;

  out << offset << "Columns(" << columns.size() << "):" << std::endl;
  for(auto& c : columns)
    c->display(out, pretty, offset+" ");
}

size_t ReportRsp::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi32(err);
  if(!err) {
    sz += Serialization::encoded_length_vi64(rgrid);  

    sz += Serialization::encoded_length_vi32(endpoints.size()); 
    for(auto& endpoint : endpoints)
      sz += Serialization::encoded_length(endpoint);

    sz += Serialization::encoded_length_vi64(columns.size());      
    for(auto c : columns)
      sz += c->encoded_length();
  }
  return sz;
}
  
void ReportRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, err);
  if(!err) {
    Serialization::encode_vi64(bufp, rgrid);

    Serialization::encode_vi32(bufp, endpoints.size());
    for(auto& endpoint : endpoints)
      Serialization::encode(endpoint, bufp);

    Serialization::encode_vi64(bufp, columns.size());
    for(auto c : columns)
      c->encode(bufp);
  }
}
  
void ReportRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  if(!(err = Serialization::decode_vi32(bufp, remainp))) {
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


}}}}

/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsColumn.h"


namespace SWC { namespace DB { namespace Specs {


Column::Ptr Column::make_ptr(int64_t cid, uint32_t reserve){
  return std::make_shared<Column>(cid, reserve);
}

Column::Ptr Column::make_ptr(int64_t cid, const Intervals& intervals){
  return std::make_shared<Column>(cid, intervals);
}

Column::Ptr Column::make_ptr(const uint8_t **bufp, size_t *remainp){
  return std::make_shared<Column>(bufp, remainp);
}

Column::Ptr Column::make_ptr(const Column& other){
  return std::make_shared<Column>(other);
}

Column::Ptr Column::make_ptr(Column::Ptr other){
  return std::make_shared<Column>(*other.get());
}

Column::Column(int64_t cid, uint32_t reserve)
                : cid(cid), intervals(0) {
  intervals.reserve(reserve);
}

Column::Column(int64_t cid, const Intervals& intervals)
                : cid(cid), intervals(intervals) {}

Column::Column(const uint8_t **bufp, size_t *remainp) {
  decode_internal(encoding_version(), bufp, remainp); 
}

Column::Column(const Column& other) {
  copy(other);
}

void Column::copy(const Column &other) {
  cid = other.cid;
  free();
  intervals.resize(other.intervals.size());
  int i = 0;
  for(const auto& intval : other.intervals)
    intervals[i++] = Interval::make_ptr(intval);
}

Column::~Column(){
  free();
}

void Column::free() {
  intervals.clear();
  /*
  while(!intervals.empty()){
    auto it = intervals.begin();
    auto p = *it;
    intervals.erase(it);
    auto interval = *p; //delete p;
  }
  */
}

bool Column::equal(const Column &other) {
  if(cid != other.cid || intervals.size() != other.intervals.size())
    return false;

  auto it2=other.intervals.begin();
  for(auto it1=intervals.begin(); it1 < intervals.end(); it1++, it2++)
    if(!(*it1)->equal(*(*it2)))
      return false;
  return true;
}

uint8_t Column::encoding_version() const {
  return 1;
}

size_t Column::encoded_length_internal() const {
  size_t len = Serialization::encoded_length_vi64(cid)
              + Serialization::encoded_length_vi32(intervals.size());
  for(auto& intval : intervals)
    len += intval->encoded_length(); 
  return len;
}

void Column::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi32(bufp, (uint32_t)intervals.size());
  for(auto& intval : intervals)
    intval->encode(bufp);
}

void Column::decode_internal(uint8_t version, const uint8_t **bufp,
	                		       size_t *remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
  uint32_t sz = Serialization::decode_vi32(bufp, remainp);
  free();
  intervals.resize(sz);
  for (auto i=0;i<sz;++i)
    intervals[i] = Interval::make_ptr(bufp, remainp);
}

const std::string Column::to_string() {
  std::string s("Column(cid=");
  s.append(std::to_string(cid));
  s.append(" intervals=[");
  for(auto& intval : intervals){
    s.append(intval->to_string());
    s.append(", ");
  }
  s.append("])");
  return s;
}

void Column::display(std::ostream& out, bool pretty, 
                     std::string offset) const {
  out << offset << "Column(\n"
      << offset << " cid=" << cid << " size=" << intervals.size() << "\n"
      << offset << " intervals=[\n"; 
  for(auto& intval : intervals)
    intval->display(out, pretty, offset+"  ");
  out << offset << " ]\n"
      << offset << ")\n"; 
}


}}}

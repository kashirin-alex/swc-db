/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsColumn.h"


namespace SWC { namespace DB { namespace Specs {


Column::Ptr Column::make_ptr(cid_t cid, uint32_t reserve){
  return std::make_shared<Column>(cid, reserve);
}

Column::Ptr Column::make_ptr(cid_t cid, const Intervals& intervals){
  return std::make_shared<Column>(cid, intervals);
}

Column::Ptr Column::make_ptr(const uint8_t** bufp, size_t* remainp){
  return std::make_shared<Column>(bufp, remainp);
}

Column::Ptr Column::make_ptr(const Column& other){
  return std::make_shared<Column>(other);
}

Column::Ptr Column::make_ptr(Column::Ptr other){
  return std::make_shared<Column>(*other.get());
}

Column::Column(cid_t cid, uint32_t reserve)
                : cid(cid), intervals(0) {
  intervals.reserve(reserve);
}

Column::Column(cid_t cid, const Intervals& intervals)
                : cid(cid), intervals(intervals) {}

Column::Column(const uint8_t** bufp, size_t* remainp) {
  decode(bufp, remainp);
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
    intervals[i++] = Interval::make_ptr(*intval.get());
}

Column::~Column() {
  free();
}

void Column::free() {
  intervals.clear();
}

bool Column::equal(const Column &other) const noexcept {
  if(cid != other.cid || intervals.size() != other.intervals.size())
    return false;

  auto it2=other.intervals.begin();
  for(auto it1=intervals.begin(); it1 < intervals.end(); ++it1, ++it2)
    if(!(*it1)->equal(*(*it2)))
      return false;
  return true;
}

size_t Column::encoded_length() const noexcept {
  size_t len = Serialization::encoded_length_vi64(cid)
              + Serialization::encoded_length_vi32(intervals.size());
  for(auto& intval : intervals)
    len += intval->encoded_length();
  return len;
}

void Column::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi32(bufp, intervals.size());
  for(auto& intval : intervals)
    intval->encode(bufp);
}

void Column::decode(const uint8_t** bufp, size_t* remainp) {
  free();
  cid = Serialization::decode_vi64(bufp, remainp);
  intervals.resize(Serialization::decode_vi32(bufp, remainp));
  for(auto& intval : intervals)
    intval = Interval::make_ptr(bufp, remainp);
}

std::string Column::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Column::print(std::ostream& out) const {
  out << "Column(cid=" << cid
      << " intervals=[";
  for(auto& intval : intervals){
    intval->print(out);
    out << ", ";
  }
  out << "])";
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

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsColumn.h"


namespace SWC { namespace DB { namespace Specs {



void Column::copy(const Column &other) {
  cid = other.cid;
  clear();
  resize(other.size());
  int i = 0;
  for(const auto& intval : other)
    (*this)[i++].reset(new Interval(*intval.get()));
}

Interval::Ptr& Column::add(Types::Column col_type) {
  return emplace_back(new Interval(col_type));
}

bool Column::equal(const Column &other) const noexcept {
  if(cid != other.cid || size() != other.size())
    return false;

  auto it2=other.cbegin();
  for(auto it1=cbegin(); it1 != cend(); ++it1, ++it2)
    if(!(*it1)->equal(*(*it2)))
      return false;
  return true;
}

size_t Column::encoded_length() const noexcept {
  size_t len = Serialization::encoded_length_vi64(cid)
              + Serialization::encoded_length_vi32(size());
  for(auto& intval : *this)
    len += intval->encoded_length();
  return len;
}

void Column::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi32(bufp, size());
  for(auto& intval : *this)
    intval->encode(bufp);
}

void Column::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  cid = Serialization::decode_vi64(bufp, remainp);
  resize(Serialization::decode_vi32(bufp, remainp));
  for(auto& intval : *this)
    intval.reset(new Interval(bufp, remainp));
}

std::string Column::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Column::print(std::ostream& out) const {
  out << "Column(cid=" << cid
      << " intervals=[";
  for(auto& intval : *this) {
    intval->print(out);
    out << ", ";
  }
  out << "])";
}

void Column::display(std::ostream& out, bool pretty,
                     std::string offset) const {
  out << offset << "Column(\n"
      << offset << " cid=" << cid << " size=" << size() << "\n"
      << offset << " intervals=[\n";
  for(auto& intval : *this)
    intval->display(out, pretty, offset+"  ");
  out << offset << " ]\n"
      << offset << ")\n";
}


}}}

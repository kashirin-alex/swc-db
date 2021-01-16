/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsValues.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


void Values::copy(const Values& other) {
  free();
  resize(other.size());
  auto it = begin();
  for(auto it2 = other.begin(); it < end(); ++it, ++it2)
    it->copy(*it2);
}

void Values::free() {
  clear();
}

Value& Values::add() {
  return emplace_back(col_type);
}

Value& Values::add(Value&& other) {
  return emplace_back(other);
}

size_t Values::size_of_internal() const {
  size_t sz = 0;
  for(auto& value : *this)
    sz += sizeof(value) + value.size;
  return sz;
}

bool Values::equal(const Values& other) const {
  if(size() == other.size()) {
    auto it = begin();
    for(auto it2 = other.begin(); it < end(); ++it, ++it2)
      if(!it->equal(*it2))
        return false;
  }
  return true;
}

bool Values::is_matching(const Cells::Cell& cell) const {
  for(auto& value : *this) {
    if(!value.is_matching(cell))
      return false;
  }
  return true;
}

size_t Values::encoded_length() const {
  size_t sz = 0;
  size_t c = 0;
  for(auto& value : *this) {
    if(value.comp != Condition::NONE) {
      ++c;
      sz += value.encoded_length();
    }
  }
  return Serialization::encoded_length_vi64(c) + sz;
}

void Values::encode(uint8_t** bufp) const {
  size_t c = 0;
  for(auto& value : *this) {
    if(value.comp != Condition::NONE)
      ++c;
  }
  Serialization::encode_vi64(bufp, c);
  for(auto& value : *this) {
    if(value.comp != Condition::NONE)
      value.encode(bufp);
  }
}

void Values::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  resize(Serialization::decode_vi64(bufp, remainp));
  for(auto& value : *this)
    value.decode(bufp, remainp);
}

void Values::print(std::ostream& out) const {
  out << "Values(";
  if(!empty()) {
    out << "size=" << size() << " [";
    for(auto& value : *this) {
      value.print(out);
      out << ", ";
    }
    out << ']';
  }
  out << ')';
}

void Values::display(std::ostream& out, bool pretty, 
                     const std::string& offset) const {
  out << offset << "Values([\n";
  for(auto& value : *this) {
    value.display(out << offset << " Value(", pretty);
    out << ")\n";
  }
  out << offset << "])\n";
}


}}}

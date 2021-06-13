/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsValues.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {



Values::Values(const Values& other)
              : Vec(other), col_type(other.col_type) {
}

void Values::copy(const Values& other) {
  Vec::operator=(other);
  col_type = other.col_type;
}

Value& Values::add(Condition::Comp comp) {
  return emplace_back(true, comp);
}

Value& Values::add(Value&& other) {
  return emplace_back(std::move(other));
}

bool Values::equal(const Values& other) const noexcept {
  if(col_type == other.col_type && size() == other.size()) {
    auto it = cbegin();
    for(auto it2 = other.cbegin(); it != cend(); ++it, ++it2)
      if(!it->equal(*it2))
        return false;
  }
  return true;
}

void Values::print(std::ostream& out) const {
  out << "Values(";
  if(!empty()) {
    out << "size=" << size() << " [";
    for(const auto& value : *this) {
      value.print(col_type, out);
      out << ", ";
    }
    out << ']';
  }
  out << ')';
}

void Values::display(std::ostream& out, bool pretty,
                     const std::string& offset) const {
  out << offset << "Values([\n";
  for(const auto& value : *this) {
    value.display(col_type, out << offset << " Value(", pretty);
    out << ")\n";
  }
  out << offset << "])\n";
}


}}}

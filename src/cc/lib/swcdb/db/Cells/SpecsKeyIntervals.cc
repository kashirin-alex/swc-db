/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsKeyIntervals.h"
#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {



KeyIntervals::KeyIntervals(const KeyIntervals& other)
                           : Vec(other) {
}

void KeyIntervals::copy(const KeyIntervals& other) {
  Vec::operator=(other);
}

KeyInterval& KeyIntervals::add() {
  return emplace_back();
}

KeyInterval& KeyIntervals::add(const KeyInterval& other) {
  return emplace_back(other);
}

KeyInterval& KeyIntervals::add(KeyInterval&& other) {
  return emplace_back(std::move(other));
}

KeyInterval& KeyIntervals::add(const Key& start, const Key& finish) {
  return emplace_back(start, finish);
}

KeyInterval& KeyIntervals::add(Key&& start, Key&& finish) {
  return emplace_back(std::move(start), std::move(finish));
}

bool KeyIntervals::equal(const KeyIntervals& other) const noexcept {
  if(size() == other.size()) {
    auto it = cbegin();
    for(auto it2 = other.cbegin(); it != cend(); ++it, ++it2)
      if(!it->start.equal(it2->start) ||
         !it->finish.equal(it2->finish))
        return false;
  }
  return true;
}

void KeyIntervals::print(std::ostream& out) const {
  out << "KeyIntervals(";
  if(!empty()) {
    out << "size=" << size();
    for(const auto& key : *this) {
      key.start.print(out << " [Start");
      key.finish.print(out << " Finish");
      out << ']';
    }
  }
  out << ')';
}

void KeyIntervals::display(std::ostream& out, bool pretty,
                            const std::string& offset) const {
  out << offset << "KeyIntervals([\n";
  for(const auto& key : *this) {
    out << offset << " Key(\n"
        << offset << "   start(";
    key.start.display(out, pretty);
    out << ")\n"
        << offset << "  finish(";
    key.finish.display(out, pretty);
    out << ")\n";
    out << offset << " )\n";
  }
  out << offset << "])\n";
}


}}}

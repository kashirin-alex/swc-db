/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsKeyIntervals.h"
#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {



KeyInterval::KeyInterval(const Key& start, const Key& finish)
                        : start(start), finish(finish) {
}

KeyInterval::KeyInterval(const uint8_t** bufp, size_t* remainp) {
  start.decode(bufp, remainp);
  finish.decode(bufp, remainp);
}

KeyInterval::KeyInterval(const KeyInterval& other)
                        : start(other.start), finish(other.finish) {
}



//KeyIntervals::KeyIntervals(const KeyIntervals& other) {
//  copy(other);
//}

void KeyIntervals::copy(const KeyIntervals& other) {
  free();
  resize(other.size());
  auto it = begin();
  for(auto it2 = other.begin(); it != end(); ++it, ++it2) {
    it->reset(new KeyInterval(*(it2->get())));
  }
}

void KeyIntervals::free() {
  clear();
}

KeyInterval::Ptr& KeyIntervals::add() {
  return emplace_back(new KeyInterval());
}

KeyInterval::Ptr& KeyIntervals::add(const KeyInterval& other) {
  return emplace_back(new KeyInterval(other));
}

KeyInterval::Ptr& KeyIntervals::add(const Key& start, const Key& finish) {
  return emplace_back(new KeyInterval(start, finish));
}

size_t KeyIntervals::size_of_internal() const noexcept {
  size_t sz = 0;
  for(auto& key : *this) {
    sz += sizeof(key);
    sz += key->start.size_of_internal();
    sz += key->finish.size_of_internal();
  }
  return sz;
}

bool KeyIntervals::equal(const KeyIntervals& other) const noexcept {
  if(size() == other.size()) {
    auto it = begin();
    for(auto it2 = other.begin(); it != end(); ++it, ++it2)
      if(!(*it)->start.equal((*it2)->start) ||
         !(*it)->finish.equal((*it2)->finish))
        return false;
  }
  return true;
}

bool KeyIntervals::is_matching(const Types::KeySeq key_seq,
                               const DB::Cell::Key& cellkey) const {
  switch(key_seq) {
    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      for(auto& key : *this) {
        if(!key->start.is_matching_lexic(cellkey) ||
           !key->finish.is_matching_lexic(cellkey))
          return false;
      }
      return true;
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      for(auto& key : *this) {
        if(!key->start.is_matching_volume(cellkey) ||
           !key->finish.is_matching_volume(cellkey))
          return false;
      }
      return true;
    default:
      return false;
  }
}

bool KeyIntervals::is_matching_start(const Types::KeySeq key_seq,
                                     const DB::Cell::Key& cellkey) const {
  switch(key_seq) {
    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      for(auto& key : *this) {
        if(!key->start.is_matching_lexic(cellkey))
          return false;
      }
      return true;
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      for(auto& key : *this) {
        if(!key->start.is_matching_volume(cellkey))
          return false;
      }
      return true;
    default:
      return false;
  }
}

size_t KeyIntervals::encoded_length() const noexcept {
  size_t sz = Serialization::encoded_length_vi64(size());
  for(auto& key : *this) {
    sz += key->start.encoded_length();
    sz += key->finish.encoded_length();
  }
  return sz;
}

void KeyIntervals::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, size());
  for(auto& key : *this) {
    key->start.encode(bufp);
    key->finish.encode(bufp);
  }
}

void KeyIntervals::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  resize(Serialization::decode_vi64(bufp, remainp));
  for(auto& key : *this) {
    key.reset(new KeyInterval(bufp, remainp));
  }
}

void KeyIntervals::print(std::ostream& out) const {
  out << "KeyIntervals(";
  if(!empty()) {
    out << "size=" << size();
    for(auto& key : *this) {
      key->start.print(out << " [Start");
      key->finish.print(out << " Finish");
      out << ']';
    }
  }
  out << ')';
}

void KeyIntervals::display(std::ostream& out, bool pretty,
                            const std::string& offset) const {
  out << offset << "KeyIntervals([\n";
  for(auto& key : *this) {
    out << offset << " Key(\n"
        << offset << "   start(";
    key->start.display(out, pretty);
    out << ")\n"
        << offset << "  finish(";
    key->finish.display(out, pretty);
    out << ")\n";
    out << offset << " )\n";
  }
  out << offset << "])\n";
}


}}}

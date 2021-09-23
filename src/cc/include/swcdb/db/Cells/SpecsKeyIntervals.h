/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsKeyIntervals_h
#define swcdb_db_cells_SpecsKeyIntervals_h


#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Cells/SpecsKey.h"


namespace SWC { namespace DB { namespace Specs {


struct KeyInterval {
  Key  start, finish;

  SWC_CAN_INLINE
  KeyInterval() noexcept { }

  SWC_CAN_INLINE
  KeyInterval(const KeyInterval& other)
              : start(other.start),
                finish(other.finish) {
  }

  SWC_CAN_INLINE
  KeyInterval(KeyInterval&& other) noexcept
              : start(std::move(other.start)),
                finish(std::move(other.finish)) {
  }

  SWC_CAN_INLINE
  KeyInterval(const Key& a_start, const Key& a_finish)
              : start(a_start), finish(a_finish) {
  }

  SWC_CAN_INLINE
  KeyInterval(Key&& a_start, Key&& a_finish) noexcept
              : start(std::move(a_start)),
                finish(std::move(a_finish)) {
  }

  SWC_CAN_INLINE
  KeyInterval(const uint8_t** bufp, size_t* remainp) {
    decode(bufp, remainp);
  }

  ~KeyInterval() noexcept;

  SWC_CAN_INLINE
  KeyInterval& operator=(const KeyInterval& other) {
    start.copy(other.start);
    finish.copy(other.finish);
    return *this;
  }

  SWC_CAN_INLINE
  KeyInterval& operator=(KeyInterval&& other) noexcept {
    start.move(other.start);
    finish.move(other.finish);
    return *this;
  }

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    return start.encoded_length() + finish.encoded_length();
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    start.encode(bufp);
    finish.encode(bufp);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    start.decode(bufp, remainp);
    finish.decode(bufp, remainp);
  }

};



class KeyIntervals : public Core::Vector<KeyInterval> {
  public:

  typedef Core::Vector<KeyInterval> Vec;
  using Vec::insert;
  using Vec::emplace_back;

  SWC_CAN_INLINE
  KeyIntervals() noexcept { }

  SWC_CAN_INLINE
  KeyIntervals(KeyIntervals&& other) noexcept
              : Vec(std::move(other)) {
  }

  ~KeyIntervals() noexcept;

  SWC_CAN_INLINE
  KeyIntervals& operator=(const KeyIntervals& other) {
    copy(other);
    return *this;
  }

  SWC_CAN_INLINE
  KeyIntervals& operator=(KeyIntervals&& other) noexcept {
    move(other);
    return *this;
  }

  SWC_CAN_INLINE
  void move(KeyIntervals& other) noexcept {
    Vec::operator=(std::move(other));
  }

  KeyIntervals(const KeyIntervals& other);

  void copy(const KeyIntervals& other);

  KeyInterval& add();

  KeyInterval& add(const KeyInterval& other);

  KeyInterval& add(KeyInterval&& other);

  KeyInterval& add(const Key& start, const Key& finish);

  KeyInterval& add(Key&& start, Key&& finish);

  size_t size_of_internal() const noexcept;

  bool SWC_PURE_FUNC equal(const KeyIntervals& other) const noexcept;

  bool is_matching(const Types::KeySeq key_seq,
                   const DB::Cell::Key& cellkey) const;

  bool is_matching_start(const Types::KeySeq key_seq,
                         const DB::Cell::Key& cellkey) const;

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    size_t sz = Serialization::encoded_length_vi64(size());
    for(const auto& key : *this)
      sz += key.encoded_length();
    return sz;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, size());
    for(const auto& key : *this)
      key.encode(bufp);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    clear();
    resize(Serialization::decode_vi64(bufp, remainp));
    for(auto& key : *this)
      key.decode(bufp, remainp);
  }

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty,
               const std::string& offset) const;

};



SWC_CAN_INLINE
size_t KeyIntervals::size_of_internal() const noexcept {
  size_t sz = 0;
  for(const auto& key : *this) {
    sz += sizeof(key);
    sz += key.start.size_of_internal();
    sz += key.finish.size_of_internal();
  }
  return sz;
}

SWC_CAN_INLINE
bool KeyIntervals::is_matching(const Types::KeySeq key_seq,
                               const DB::Cell::Key& cellkey) const {
  if(empty())
    return true;

  switch(key_seq) {
    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      for(const auto& key : *this) {
        if(!key.start.is_matching_lexic(cellkey) ||
           !key.finish.is_matching_lexic(cellkey))
          return false;
      }
      return true;
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      for(const auto& key : *this) {
        if(!key.start.is_matching_volume(cellkey) ||
           !key.finish.is_matching_volume(cellkey))
          return false;
      }
      return true;
    default:
      return false;
  }
}

SWC_CAN_INLINE
bool KeyIntervals::is_matching_start(const Types::KeySeq key_seq,
                                     const DB::Cell::Key& cellkey) const {
  switch(key_seq) {
    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      for(const auto& key : *this) {
        if(!key.start.is_matching_lexic(cellkey))
          return false;
      }
      return true;
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      for(const auto& key : *this) {
        if(!key.start.is_matching_volume(cellkey))
          return false;
      }
      return true;
    default:
      return false;
  }
}



}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsKeyIntervals.cc"
#endif


#endif // swcdb_db_cells_SpecsKeyIntervals_h

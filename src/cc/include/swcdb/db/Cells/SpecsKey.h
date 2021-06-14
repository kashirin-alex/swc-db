/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h


#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


struct Fraction final : public std::string {

  Condition::Comp comp;
  mutable void*   compiled = nullptr;

  SWC_CAN_INLINE
  Fraction() { }

  SWC_CAN_INLINE
  Fraction(std::string&& fraction, Condition::Comp comp) noexcept
          : std::string(std::move(fraction)), comp(comp) {
  }

  SWC_CAN_INLINE
  Fraction(Fraction&& other) noexcept
          : std::string(std::move(other)), comp(other.comp) {
  }

  SWC_CAN_INLINE
  Fraction& operator=(Fraction&& other) noexcept {
    std::string::operator=(std::move(other));
    comp = other.comp;
    return *this;
  }

  SWC_CAN_INLINE
  Fraction& operator=(std::string&& other) noexcept {
    std::string::operator=(std::move(other));
    return *this;
  }

  SWC_CAN_INLINE
  Fraction(const char* buf, uint32_t len, Condition::Comp comp)
          : std::string(buf, len), comp(comp) {
  }

  SWC_CAN_INLINE
  Fraction(const Fraction& other)
          : std::string(other), comp(other.comp) {
  }

  SWC_CAN_INLINE
  Fraction& operator=(const Fraction& other) {
    std::string::operator=(other);
    comp = other.comp;
    return *this;
  }

  SWC_CAN_INLINE
  ~Fraction() {
    if(compiled) switch(comp) {
      case Condition::RE:
        delete static_cast<re2::RE2*>(compiled);
        break;
      default: break;
    }
  }

  SWC_CAN_INLINE
  bool operator==(const Fraction &other) const {
    return  other.comp == comp && length() == other.length() &&
            Condition::mem_eq(
            reinterpret_cast<const uint8_t*>(data()),
            reinterpret_cast<const uint8_t*>(other.data()),
            length());
  }

  SWC_CAN_INLINE
  uint32_t encoded_length() const noexcept {
    return 1 + Serialization::encoded_length_vi32(size()) + size();
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, comp);
    Serialization::encode_vi32(bufp, size());
    if(!empty()) {
      memcpy(*bufp, data(), size());
      *bufp += size();
    }
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    clear();
    comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
    if(uint32_t len = Serialization::decode_vi32(bufp, remainp)) {
      append(reinterpret_cast<const char*>(*bufp), len);
      *bufp += len;
      *remainp -= len;
    }
  }

  void print(std::ostream& out, bool pretty=true) const;

  template<Types::KeySeq T_seq>
  bool is_matching(const uint8_t* ptr, uint32_t len) const;

};



class Key final : public std::vector<Fraction> {
  public:

  using std::vector<Fraction>::insert;

  typedef std::shared_ptr<Key> Ptr;

  SWC_CAN_INLINE
  explicit Key() noexcept { }

  SWC_CAN_INLINE
  explicit Key(Key&& other) noexcept
              : std::vector<Fraction>(std::move(other)) {
  }

  SWC_CAN_INLINE
  explicit Key(const DB::Cell::Key &cell_key, Condition::Comp comp) {
    set(cell_key, comp);
  }

  explicit Key(const Key& other);

  //~Key() { }

  size_t size_of_internal() const noexcept;

  void copy(const Key &other);

  SWC_CAN_INLINE
  Key& operator=(Key&& other) noexcept {
    move(other);
    return *this;
  }

  SWC_CAN_INLINE
  void move(Key& other) noexcept {
    std::vector<Fraction>::operator=(std::move(other));
  }

  bool equal(const Key &other) const noexcept;


  void set(const DB::Cell::Key &cell_key, Condition::Comp comp);

  SWC_CAN_INLINE
  void set(int32_t idx, Condition::Comp comp) {
    if(empty())
      return;
    (begin()+idx)->comp = comp;
  }


  Fraction& add(Fraction&& other);

  Fraction& add(std::string&& fraction, Condition::Comp comp);

  Fraction& add(const char* buf, uint32_t len, Condition::Comp comp);

  SWC_CAN_INLINE
  Fraction& add(const std::string& fraction, Condition::Comp comp) {
    return add(fraction.c_str(), fraction.length(), comp);
  }

  SWC_CAN_INLINE
  Fraction& add(const std::string_view& fraction, Condition::Comp comp) {
    return add(fraction.data(), fraction.length(), comp);
  }

  SWC_CAN_INLINE
  Fraction& add(const char* fraction, Condition::Comp comp) {
    return add(fraction, strlen(fraction), comp);
  }

  SWC_CAN_INLINE
  Fraction& add(const uint8_t* fraction, uint32_t len, Condition::Comp comp) {
    return add(reinterpret_cast<const char*>(fraction), len, comp);
  }


  Fraction& insert(uint32_t idx, Fraction&& other);

  Fraction& insert(uint32_t idx, std::string&& fraction,
                   Condition::Comp comp);

  Fraction& insert(uint32_t idx, const char* buf, uint32_t len,
                   Condition::Comp comp);

  SWC_CAN_INLINE
  Fraction& insert(uint32_t idx, const std::string& fraction,
                   Condition::Comp comp) {
    return insert(idx, fraction.c_str(), fraction.length(), comp);
  }

  SWC_CAN_INLINE
  Fraction& insert(uint32_t idx, const std::string_view& fraction,
                   Condition::Comp comp) {
    return insert(idx, fraction.data(), fraction.length(), comp);
  }

  SWC_CAN_INLINE
  Fraction& insert(uint32_t idx, const uint8_t* fraction, uint32_t len,
                   Condition::Comp comp) {
    return insert(idx, reinterpret_cast<const char*>(fraction), len, comp);
  }

  SWC_CAN_INLINE
  Fraction& insert(uint32_t idx, const char* fraction, Condition::Comp comp)  {
    return insert(idx, fraction, strlen(fraction), comp);
  }

  SWC_CAN_INLINE
  std::string_view get(const uint32_t idx, Condition::Comp& comp) const {
    auto& f = (*this)[idx];
    comp = f.comp;
    return f;
  }

  SWC_CAN_INLINE
  std::string_view get(const uint32_t idx) const {
    return (*this)[idx];
  }

  void get(DB::Cell::Key& key) const;


  void remove(uint32_t idx, bool recursive=false);


  SWC_CAN_INLINE
  uint32_t encoded_length() const noexcept {
    uint32_t len = Serialization::encoded_length_vi32(size());
    for(auto it = begin(); it != end(); ++it)
      len += it->encoded_length();
    return len;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_vi32(bufp, size());
    for(auto it = begin(); it != end(); ++it)
      it->encode(bufp);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    clear();
    resize(Serialization::decode_vi32(bufp, remainp));
    for(auto it = begin(); it != end(); ++it)
      it->decode(bufp, remainp);
  }


  SWC_CAN_INLINE
  bool is_matching(const Types::KeySeq seq, const Cell::Key &key) const {
    switch(seq) {
      case Types::KeySeq::LEXIC:
      case Types::KeySeq::FC_LEXIC:
        return is_matching_lexic(key);
      case Types::KeySeq::VOLUME:
      case Types::KeySeq::FC_VOLUME:
        return is_matching_volume(key);
      default:
        return false;
    }
  }

  bool is_matching_lexic(const Cell::Key &key) const;

  bool is_matching_volume(const Cell::Key &key) const;

  template<Types::KeySeq T_seq>
  bool is_matching(const Cell::Key &key) const;

  std::string to_string() const;

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=true) const;

};



template<Types::KeySeq T_seq>
SWC_CAN_INLINE
bool
Fraction::is_matching(const uint8_t* ptr, uint32_t len) const {
  switch(comp) {
    case Condition::RE: {
      if(empty())
        return !ptr|| !len;
      if(!compiled)
        compiled = new re2::RE2(re2::StringPiece(data(), size()));
      return Condition::re(
        *static_cast<re2::RE2*>(compiled),
        reinterpret_cast<const char*>(ptr), len
      );
    }
    default:
      return KeySeq::is_matching<T_seq>(
        comp, reinterpret_cast<const uint8_t*>(c_str()), size(), ptr, len);
  }
}



SWC_CAN_INLINE
size_t Key::size_of_internal() const noexcept {
  size_t sz = 0;
  for(auto& f : *this) {
    sz += sizeof(f);
    sz += f.size();
  }
  return sz;
}

SWC_CAN_INLINE
bool Key::is_matching_lexic(const Cell::Key &key) const {
  return is_matching<Types::KeySeq::LEXIC>(key);
}

SWC_CAN_INLINE
bool Key::is_matching_volume(const Cell::Key &key) const {
  return is_matching<Types::KeySeq::VOLUME>(key);
}

template<Types::KeySeq T_seq>
SWC_CAN_INLINE
bool
Key::is_matching(const Cell::Key &key) const {
  if(empty())
    return true;
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = key.data;
  uint32_t len;
  auto it = begin();
  for(uint24_t c = key.count; c && it != end(); ++it, --c, ptr += len) {
    len = Serialization::decode_vi24(&ptr);
    if(!it->is_matching<T_seq>(ptr, len))
      return false;
    comp = it->comp;
  }
  if(size() == key.count || // [,,>=''] spec incl. prior-match
     (size() == key.count + 1 && it->empty() && it->comp == Condition::GE))
    return true;

  switch(comp) {
    case Condition::LT:
    case Condition::LE:
      return empty() || size() > key.count;
    case Condition::GT:
      return empty() || size() < key.count;
    case Condition::GE:
      return empty() || size() < key.count;
    case Condition::PF:
    case Condition::RE:
      return size() < key.count;
    case Condition::NE:
    case Condition::NONE:
      return true;
    default: // Condition::EQ:
      return false;
  }
}



}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsKey.cc"
#endif

#endif // swcdb_db_cells_SpecsKey_h

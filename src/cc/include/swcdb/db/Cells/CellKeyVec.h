/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellKeyVec_h
#define swcdb_db_cells_CellKeyVec_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cell {


typedef std::vector<std::basic_string<uint8_t>> VecFraction;


class KeyVec final : public VecFraction {
  public:

  typedef VecFraction::value_type Fraction;

  using VecFraction::size;

  SWC_CAN_INLINE
  explicit KeyVec() noexcept { }

  SWC_CAN_INLINE
  explicit KeyVec(KeyVec&& other) noexcept
                  : VecFraction(std::move(other)) {
  }

  explicit KeyVec(const KeyVec& other);

  //~KeyVec() { }

  SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    size_t sz = 0;
    for(auto& f : *this) {
      sz += sizeof(f);
      sz += f.length();
    }
    return sz;
  }

  KeyVec& operator=(const KeyVec&) = delete;

  void copy(const KeyVec &other);

  bool equal(const KeyVec &other) const noexcept;

  SWC_CAN_INLINE
  void add(const Fraction& fraction) {
    add(fraction.data(), fraction.length());
  }

  SWC_CAN_INLINE
  void add(const std::string& fraction) {
    add(fraction.c_str(), fraction.length());
  }

  SWC_CAN_INLINE
  void add(const char* fraction) {
    add(fraction, strlen(fraction));
  }

  SWC_CAN_INLINE
  void add(const char* fraction, const uint32_t len)  {
    add(reinterpret_cast<const uint8_t*>(fraction), len);
  }

  void add(const uint8_t* fraction, const uint32_t len);

  SWC_CAN_INLINE
  void insert(const uint32_t idx, const Fraction& fraction) {
    insert(idx, fraction.data(), fraction.length());
  }

  SWC_CAN_INLINE
  void insert(const uint32_t idx, const std::string& fraction) {
    insert(idx, fraction.c_str(), fraction.length());
  }

  SWC_CAN_INLINE
  void insert(const uint32_t idx, const char* fraction) {
    insert(idx, fraction, strlen(fraction));
  }

  SWC_CAN_INLINE
  void insert(const uint32_t idx, const char* fraction, const uint32_t len) {
    insert(idx, reinterpret_cast<const uint8_t*>(fraction), len);
  }

  void insert(const uint32_t idx, const uint8_t* fraction, const uint32_t len);

  SWC_CAN_INLINE
  void set(const uint32_t idx, const Fraction& fraction) {
    set(idx, fraction.data(), fraction.length());
  }

  SWC_CAN_INLINE
  void set(const uint32_t idx, const std::string& fraction) {
    set(idx, fraction.c_str(), fraction.length());
  }

  SWC_CAN_INLINE
  void set(const uint32_t idx, const char* fraction) {
    set(idx, fraction, strlen(fraction));
  }

  SWC_CAN_INLINE
  void set(const uint32_t idx, const char* fraction, const uint32_t len) {
    set(idx, reinterpret_cast<const uint8_t*>(fraction), len);
  }

  void set(const uint32_t idx, const uint8_t* fraction, const uint32_t len);

  void remove(const uint32_t idx);

  SWC_CAN_INLINE
  const Fraction& get(const uint32_t idx) const {
    return (*this)[idx];
  }

  SWC_CAN_INLINE
  void get(const uint32_t idx, Fraction& fraction) const {
    fraction = (*this)[idx];
  }

  SWC_CAN_INLINE
  void get(const uint32_t idx, std::string& fraction) const {
    fraction.assign(
      reinterpret_cast<const char*>((*this)[idx].data()),
      (*this)[idx].size()
    );
  }

  uint32_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  std::string to_string() const;

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, const KeyVec& key) {
    key.print(out);
    return out;
  }

};



SWC_CAN_INLINE
KeyVec::KeyVec(const KeyVec& other)
               : VecFraction(other) {
}

SWC_CAN_INLINE
void KeyVec::copy(const KeyVec &other) {
  clear();
  assign(other.cbegin(), other.cend());
}

SWC_CAN_INLINE
bool KeyVec::equal(const KeyVec &other) const noexcept {
  return *this == other;
}

SWC_CAN_INLINE
void KeyVec::add(const uint8_t* fraction, const uint32_t len) {
  emplace_back(fraction, len);
}

SWC_CAN_INLINE
void KeyVec::insert(const uint32_t idx, const uint8_t* fraction,
                    const uint32_t len) {
  emplace(cbegin() + idx, fraction, len);
}

SWC_CAN_INLINE
void KeyVec::set(const uint32_t idx, const uint8_t* fraction,
                 const uint32_t len) {
  (*this)[idx].assign(fraction, len);
}

SWC_CAN_INLINE
void KeyVec::remove(const uint32_t idx) {
  if(idx < size())
    erase(cbegin() + idx);
}

SWC_CAN_INLINE
uint32_t KeyVec::encoded_length() const noexcept {
  uint32_t len = Serialization::encoded_length_vi32(size());
  for(auto it = cbegin(); it != cend(); ++it)
    len += Serialization::encoded_length_vi32(it->length()) + it->length();
  return len;
}

SWC_CAN_INLINE
void KeyVec::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, size());
  uint32_t len;
  for(auto it = cbegin(); it != cend(); ++it) {
    Serialization::encode_vi32(bufp, len = it->length());
    memcpy(*bufp, it->data(), len);
    *bufp += len;
  }
}

SWC_CAN_INLINE
void KeyVec::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  resize(Serialization::decode_vi32(bufp, remainp));
  uint32_t len;
  for(auto it = begin(); it != cend(); ++it) {
    *remainp -= len = Serialization::decode_vi32(bufp, remainp);
    it->assign(*bufp, len);
    *bufp += len;
  }
}



}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellKeyVec.cc"
#endif

#endif // swcdb_db_Cells_CellKeyVec_h

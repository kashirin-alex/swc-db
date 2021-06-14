/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellKey_h
#define swcdb_db_cells_CellKey_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/CellKeyVec.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB {


//! The SWC-DB Cell C++ namespace 'SWC::DB::Cell'
namespace Cell {


class Key final {
  public:

  typedef std::shared_ptr<Key> Ptr;

  SWC_CAN_INLINE
  explicit Key(bool own = true) noexcept
              : own(own), count(0), size(0), data(nullptr) {
  }

  explicit Key(const Key& other);

  explicit Key(const Key& other, bool own);

  SWC_CAN_INLINE
  Key(Key&& other) noexcept
        : own(other.own), count(other.count), size(other.size),
          data(other.data) {
    other.data = nullptr;
    other.size = 0;
    other.count = 0;
  }

  Key& operator=(const Key&) = delete;

  SWC_CAN_INLINE
  Key& operator=(Key&& other) noexcept {
    move(other);
    return *this;
  }

  void move(Key& other) noexcept;

  void copy(const Key& other);

  void copy(uint24_t after_idx, const Key& other);

  SWC_CAN_INLINE
  ~Key() {
    _free();
  }

  SWC_CAN_INLINE
  void _free() {
    if(own && data)
      delete [] data;
  }

  SWC_CAN_INLINE
  void free() {
    _free();
    data = nullptr;
    size = 0;
    count = 0;
  }

  SWC_CAN_INLINE
  bool sane() const noexcept {
    return (count && size && data) || (!count && !size && !data);
  }

  SWC_CAN_INLINE
  void add(const std::string_view& fraction) {
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
  void add(const char* fraction, uint32_t len) {
    add(reinterpret_cast<const uint8_t*>(fraction), len);
  }

  void add(const uint8_t* fraction, uint24_t len);

  template<typename T>
  void add(const T cbegin, const T cend);

  void add(const std::vector<std::string>& fractions);

  void add(const std::vector<KeyVec::Fraction>& fractions);

  SWC_CAN_INLINE
  void insert(uint32_t idx, const std::string& fraction) {
    insert(idx, fraction.c_str(), fraction.length());
  }

  SWC_CAN_INLINE
  void insert(uint32_t idx, const char* fraction) {
    insert(idx, fraction, strlen(fraction));
  }

  SWC_CAN_INLINE
  void insert(uint32_t idx, const char* fraction, uint32_t len) {
    insert(idx, reinterpret_cast<const uint8_t*>(fraction), len);
  }

  void insert(uint32_t idx, const uint8_t* fraction, uint24_t len);

  void remove(uint32_t idx, bool recursive=false);

  SWC_CAN_INLINE
  std::string get_string(uint32_t idx) const {
    const char* fraction = nullptr;
    uint32_t length = 0;
    get(idx, &fraction, &length);
    return std::string(fraction, length);
  }

  void get(uint32_t idx, const char** fraction, uint32_t* length) const;

  SWC_CAN_INLINE
  bool equal(const Key& other) const noexcept {
    return count == other.count &&
          ((!data && !other.data) ||
           Condition::eq(data, size, other.data, other.size));
  }

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return !count;
  }

  uint32_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp, bool owner);

  void convert_to(std::vector<std::string>& key) const;

  void convert_to(std::vector<KeyVec::Fraction>& key) const;

  void read(const std::vector<std::string>& key);

  bool equal(const std::vector<std::string>& key) const;

  std::string to_string() const;

  void display(std::ostream& out, bool pretty=true,
               const char* sep = ",") const;

  void display_details(std::ostream& out, bool pretty=true) const;

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, const Key& key) {
    key.print(out);
    return out;
  }

  bool      own;
  uint24_t  count;
  uint32_t  size;
  uint8_t*  data;

  private:

  SWC_CAN_INLINE
  uint8_t* _data(const uint8_t* ptr) {
    return size
      ? static_cast<uint8_t*>(memcpy(new uint8_t[size], ptr, size))
      : nullptr;
  }

};



SWC_CAN_INLINE
Key::Key(const Key& other)
        : own(other.size), count(other.count), size(other.size),
          data(_data(other.data)) {
}

SWC_CAN_INLINE
Key::Key(const Key& other, bool own)
        : own(own), count(other.count), size(other.size),
          data(own ? _data(other.data): other.data) {
}

SWC_CAN_INLINE
void Key::move(Key& other) noexcept {
  _free();
  own =  other.own;
  size = other.size;
  count = other.count;
  data = other.data;
  other.data = nullptr;
  other.size = 0;
  other.count = 0;
}

SWC_CAN_INLINE
void Key::copy(const Key& other) {
  _free();
  own = true;
  size = other.size;
  count = other.count;
  data = _data(other.data);
}

SWC_CAN_INLINE
uint32_t Key::encoded_length() const noexcept {
  return Serialization::encoded_length_vi24(count) + size;
}

SWC_CAN_INLINE
void Key::encode(uint8_t** bufp) const {
  Serialization::encode_vi24(bufp, count);
  if(size) {
    memcpy(*bufp, data, size);
    *bufp += size;
  }
}

SWC_CAN_INLINE
void Key::decode(const uint8_t** bufp, size_t* remainp, bool owner) {
  _free();
  if((count = Serialization::decode_vi24(bufp, remainp))) {
    uint24_t n=count;
    const uint8_t* ptr_start = *bufp;
    do *bufp += Serialization::decode_vi24(bufp);
    while(--n);
    *remainp -= (size = *bufp - ptr_start);
    data = (own = owner) ? _data(ptr_start) : const_cast<uint8_t*>(ptr_start);
  } else {
    own = owner;
    data = nullptr;
    size = 0;
  }
}



}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellKey.cc"
#endif

#endif // swcdb_db_Cells_CellKey_h

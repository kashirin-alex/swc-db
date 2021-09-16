/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsValue_h
#define swcdb_db_cells_SpecsValue_h


#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Specs {

class Value {
  public:

  constexpr SWC_CAN_INLINE
  explicit Value(bool a_own=true,
                 Condition::Comp a_comp=Condition::NONE) noexcept
                : own(a_own), comp(a_comp),
                  size(0), data(nullptr), matcher(nullptr) {
  }

  SWC_CAN_INLINE
  explicit Value(const char* data_n, Condition::Comp comp_n,
                 bool owner=false)
                : own(false), matcher(nullptr) {
    set(data_n, strlen(data_n), comp_n, owner);
  }

  SWC_CAN_INLINE
  explicit Value(const char* data_n, const uint32_t size_n,
                 Condition::Comp comp_n, bool owner=false)
                : own(false), matcher(nullptr) {
    set(data_n, size_n, comp_n, owner);
  }

  SWC_CAN_INLINE
  explicit Value(const uint8_t* data_n, const uint32_t size_n,
                 Condition::Comp comp_n, bool owner=false)
                : own(false), matcher(nullptr) {
    set(data_n, size_n, comp_n, owner);
  }

  SWC_CAN_INLINE
  explicit Value(int64_t count, Condition::Comp comp_n)
                : own(false), matcher(nullptr) {
    set_counter(count, comp_n);
  }

  SWC_CAN_INLINE
  explicit Value(const Value &other)
                : own(false), matcher(nullptr) {
    copy(other);
  }

  SWC_CAN_INLINE
  Value& operator=(const Value& other) {
    copy(other);
    return *this;
  }

  SWC_CAN_INLINE
  Value& operator=(Value&& other) noexcept {
    move(other);
    return *this;
  }

  SWC_CAN_INLINE
  void copy(const Value &other) {
    set(other.data, other.size, other.comp, true);
  }

  constexpr SWC_CAN_INLINE
  explicit Value(Value&& other) noexcept
                : own(other.own), comp(other.comp),
                  size(other.size), data(other.data),
                  matcher(other.matcher) {
    other.comp = Condition::NONE;
    other.data = nullptr;
    other.matcher = nullptr;
    other.size = 0;
  }

  SWC_CAN_INLINE
  void set(const char* data_n, Condition::Comp comp_n, bool owner=true) {
    set(data_n, strlen(data_n), comp_n, owner);
  }

  SWC_CAN_INLINE
  void set(const std::string& data_n, Condition::Comp comp_n) {
    set(data_n.c_str(), data_n.length(), comp_n, true);
  }

  SWC_CAN_INLINE
  void set(const char* data_n, uint32_t size_n,
           Condition::Comp comp_n, bool owner=true) {
    set(reinterpret_cast<const uint8_t*>(data_n), size_n, comp_n, owner);
  }

  void move(Value &other) noexcept;

  void set_counter(int64_t count, Condition::Comp comp_n);

  void set(const uint8_t* data_n, const uint32_t size_n,
           Condition::Comp comp_n, bool owner=false);

  SWC_CAN_INLINE
  ~Value() {
    _free();
  }

  SWC_CAN_INLINE
  void _free() {
    if(own && data)
      delete [] data;
    if(matcher)
      delete matcher;
  }

  SWC_CAN_INLINE
  void free() {
    _free();
    data = nullptr;
    size = 0;
    matcher = nullptr;
  }

  constexpr SWC_CAN_INLINE
  bool empty() const noexcept {
    return comp == Condition::NONE;
  }

  constexpr
  bool equal(const Value &other) const noexcept;

  constexpr SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    size_t sz = 1;
    if(comp != Condition::NONE) {
      sz += Serialization::encoded_length_vi32(size);
      sz += size;
    }
    return sz;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, comp);
    if(comp != Condition::NONE) {
      Serialization::encode_vi32(bufp, size);
      memcpy(*bufp, data, size);
      *bufp+=size;
    }
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp, bool owner=false) {
    free();
    own = owner;
    comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
    if(comp != Condition::NONE) {
      if((size = Serialization::decode_vi32(bufp, remainp))) {
        data = own
          ? static_cast<uint8_t*>(memcpy(new uint8_t[size], *bufp, size))
          : const_cast<uint8_t*>(*bufp);
        *remainp -= size;
        *bufp += size;
      }
    }
  }


  SWC_CAN_INLINE
  bool is_matching(Types::Column col_type, const Cells::Cell& cell) const {
    switch(col_type) {
      case Types::Column::PLAIN:
        return is_matching_plain(cell);
      case Types::Column::SERIAL:
        return is_matching_serial(cell);
      case Types::Column::COUNTER_I64:
      case Types::Column::COUNTER_I32:
      case Types::Column::COUNTER_I16:
      case Types::Column::COUNTER_I8:
        return is_matching_counter(cell);
      default:
        return false;
    }
  }

  bool is_matching_plain(const Cells::Cell& cell) const;

  bool is_matching_serial(const Cells::Cell& cell) const;

  bool is_matching_counter(const Cells::Cell& cell) const;

  SWC_CAN_INLINE
  std::string to_string(Types::Column col_type) const {
    std::string s;
    {
      std::stringstream ss;
      print(col_type, ss);
      s = ss.str();
    }
    return s;
  }

  void print(Types::Column col_type, std::ostream& out) const;

  void display(Types::Column col_type, std::ostream& out,
               bool pretty=true) const;

  bool            own;
  Condition::Comp comp;
  uint32_t        size;
  uint8_t*        data;

  struct TypeMatcher {
    virtual ~TypeMatcher() { }
  };
  private:
  mutable TypeMatcher*  matcher;
};



SWC_CAN_INLINE
void Value::move(Value &other) noexcept {
  free();
  own = other.own;
  comp = other.comp;
  data = other.data;
  size = other.size;
  matcher = other.matcher;
  other.comp = Condition::NONE;
  other.data = nullptr;
  other.matcher = nullptr;
  other.size = 0;
}

SWC_CAN_INLINE
void Value::set_counter(int64_t count, Condition::Comp comp_n) {
  uint32_t len = Serialization::encoded_length_vi64(count);
  uint8_t data_n[10];
  uint8_t* ptr = data_n;
  Serialization::encode_vi64(&ptr, count);
  set(data_n, len, comp_n, true);
}

SWC_CAN_INLINE
void Value::set(const uint8_t* data_n, const uint32_t size_n,
                Condition::Comp comp_n, bool owner) {
  free();
  own   = owner;
  comp = comp_n;
  if((size = size_n))
    data = own
      ? static_cast<uint8_t*>(memcpy(new uint8_t[size], data_n, size))
      : const_cast<uint8_t*>(data_n);
}

constexpr SWC_CAN_INLINE
bool Value::equal(const Value &other) const noexcept {
  return
    size == other.size &&
    ((!data && !other.data) ||
     (data && other.data && Condition::mem_eq(data, other.data, size)));
}



}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValue.cc"
#endif

#endif // swcdb_db_cells_SpecsValue_h

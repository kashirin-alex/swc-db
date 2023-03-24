/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsTimestamp_h
#define swcdb_db_cells_SpecsTimestamp_h


#include "swcdb/core/Comparators.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


class Timestamp {
  public:

  constexpr SWC_CAN_INLINE
  explicit Timestamp() noexcept
                    : value(0), comp(Condition::NONE), was_set(false) {
  }

  constexpr SWC_CAN_INLINE
  explicit Timestamp(int64_t timestamp, Condition::Comp a_comp) noexcept
                    : value(timestamp), comp(a_comp), was_set(true) {
  }

  SWC_CAN_INLINE
  explicit Timestamp(const uint8_t** bufp, size_t* remainp)
    : value(),
      comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
      was_set(true) {
    value = comp == Condition::NONE
      ? 0
      : Serialization::decode_i64(bufp, remainp);
  }

  ~Timestamp() noexcept { }

  constexpr SWC_CAN_INLINE
  void copy(const Timestamp &other) noexcept {
    set(other.value, other.comp);
  }

  constexpr SWC_CAN_INLINE
  void set(int64_t timestamp, Condition::Comp comperator) noexcept {
    value = timestamp;
    comp  = comperator;
    was_set = true;
  }

  constexpr SWC_CAN_INLINE
  void free() noexcept {
    value  = 0;
    comp  = Condition::NONE;
    was_set = false;
  }

  constexpr SWC_CAN_INLINE
  bool empty() const noexcept {
    return !was_set;
  }

  constexpr SWC_CAN_INLINE
  bool equal(const Timestamp &other) const noexcept {
    return value == other.value && comp == other.comp;
  }

  constexpr SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    size_t sz = 1;
    if(comp != Condition::NONE)
      sz += 8;
    return sz;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, comp);
    if(comp != Condition::NONE)
      Serialization::encode_i64(bufp, value);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
    if(comp != Condition::NONE)
      value = Serialization::decode_i64(bufp, remainp);
  }

  SWC_CAN_INLINE
  bool is_matching(int64_t other) const noexcept {
    return Condition::is_matching(comp, value, other);
  }

  SWC_CAN_INLINE
  std::string to_string() const {
    std::string s;
    {
      std::stringstream ss;
      print(ss);
      s = ss.str();
    }
    return s;
  }

  void display(std::ostream& out) const;

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, const Timestamp& key) {
    key.print(out);
    return out;
  }

  int64_t          value;
  Condition::Comp  comp;
  bool             was_set;

};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsTimestamp.cc"
#endif

#endif // swcdb_db_cells_SpecsTimestamp_h

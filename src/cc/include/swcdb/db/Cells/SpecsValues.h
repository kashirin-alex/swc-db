/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsValues_h
#define swcdb_db_cells_SpecsValues_h


#include "swcdb/db/Cells/SpecsValue.h"


namespace SWC { namespace DB { namespace Specs {


class Values : public Core::Vector<Value> {
  public:

  typedef Core::Vector<Value> Vec;

  using Vec::insert;
  using Vec::emplace_back;


  Types::Column col_type;

  SWC_CAN_INLINE
  Values(Types::Column a_col_type = Types::Column::UNKNOWN) noexcept
        : col_type(a_col_type) {
  }

  SWC_CAN_INLINE
  Values(Values&& other) noexcept
        : Vec(std::move(other)), col_type(other.col_type) {
  }

  SWC_CAN_INLINE
  Values& operator=(const Values& other) {
    copy(other);
    return *this;
  }

  SWC_CAN_INLINE
  Values& operator=(Values&& other) noexcept {
    move(other);
    return *this;
  }

  SWC_CAN_INLINE
  void move(Values& other) noexcept {
    Vec::operator=(std::move(other));
    col_type = other.col_type;
  }

  Values(const Values& other);

  void copy(const Values& other);

  Value& add(Condition::Comp comp=Condition::EQ);

  Value& add(Value&& other);

  size_t size_of_internal() const noexcept;

  bool equal(const Values& other) const noexcept;

  bool is_matching(const Cells::Cell& cell) const;

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    size_t sz = 0;
    size_t c = 0;
    for(const auto& value : *this) {
      if(value.comp != Condition::NONE) {
        ++c;
        sz += value.encoded_length();
      }
    }
    return Serialization::encoded_length_vi64(c) + sz;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    size_t c = 0;
    for(const auto& value : *this) {
      if(value.comp != Condition::NONE)
        ++c;
    }
    Serialization::encode_vi64(bufp, c);
    for(const auto& value : *this) {
      if(value.comp != Condition::NONE)
        value.encode(bufp);
    }
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp, bool owner=false) {
    clear();
    resize(Serialization::decode_vi64(bufp, remainp));
    for(auto& value : *this)
      value.decode(bufp, remainp, owner);
  }

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty,
               const std::string& offset) const;

};



SWC_CAN_INLINE
size_t Values::size_of_internal() const noexcept {
  size_t sz = sizeof(*this);
  for(const auto& value : *this)
    sz += sizeof(value) + value.size;
  return sz;
}

SWC_CAN_INLINE
bool Values::is_matching(const Cells::Cell& cell) const {
  if(empty())
    return true;

  switch(col_type) {
    case Types::Column::PLAIN: {
      for(const auto& value : *this) {
        if(!value.is_matching_plain(cell))
          return false;
      }
      return true;
    }
    case Types::Column::SERIAL: {
      for(const auto& value : *this) {
        if(!value.is_matching_serial(cell))
          return false;
      }
      return true;
    }
    case Types::Column::COUNTER_I64:
    case Types::Column::COUNTER_I32:
    case Types::Column::COUNTER_I16:
    case Types::Column::COUNTER_I8: {
      for(const auto& value : *this) {
        if(!value.is_matching_counter(cell))
          return false;
      }
      return true;
    }
    default:
      return false;
  }
}



}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValues.cc"
#endif


#endif // swcdb_db_cells_SpecsValues_h

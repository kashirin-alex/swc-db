/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellValueSerialFieldUpdate_h
#define swcdb_db_cells_CellValueSerialFieldUpdate_h

#include "swcdb/db/Cells/CellValueSerialField.h"


namespace SWC { namespace DB { namespace Cell {



//! The SWC-DB Serial Cell C++ namespace 'SWC::DB::Cell::Serial'
namespace Serial {



//! The SWC-DB Serial Value Cell C++ namespace 'SWC::DB::Cell::Serial::Value'
namespace Value {



//
class FieldUpdate {
  public:
  static constexpr const uint8_t CTRL_DEFAULT       = 0x00;
  static constexpr const uint8_t CTRL_NO_ADD_FIELD  = 0x01;
  static constexpr const uint8_t CTRL_DELETE_FIELD  = 0x02;
  SWC_CAN_INLINE
  FieldUpdate() noexcept : ctrl(CTRL_DEFAULT) { }
  SWC_CAN_INLINE
  FieldUpdate(const uint8_t** ptrp, size_t* remainp)
              : ctrl(Serialization::decode_i8(ptrp, remainp)) {
  }
  virtual ~FieldUpdate() noexcept { }
  SWC_CAN_INLINE
  void set_ctrl(const char** ptr, uint32_t* remainp) noexcept {
    if(*remainp && **ptr == '!') {
      set_no_add_field();
      ++*ptr;
      --*remainp;
    } else if(*remainp >= 3 && Condition::str_eq(*ptr, "DEL", 3)) {
      set_delete_field();
      *ptr += 3;
      *remainp -=3;
    }
  }
  SWC_CAN_INLINE
  void set_no_add_field() noexcept {
    ctrl |= CTRL_NO_ADD_FIELD;
  }
  SWC_CAN_INLINE
  bool is_no_add_field() const noexcept {
    return ctrl & CTRL_NO_ADD_FIELD;
  }
  SWC_CAN_INLINE
  void set_delete_field() noexcept {
    ctrl |= CTRL_DELETE_FIELD;
  }
  bool is_delete_field() const noexcept {
    return ctrl & CTRL_DELETE_FIELD;
  }
  SWC_CAN_INLINE
  virtual uint24_t encoded_length() const noexcept {
    return 1;
  }
  SWC_CAN_INLINE
  virtual void encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, ctrl);
  }
  SWC_CAN_INLINE
  virtual void decode(const uint8_t** ptrp, size_t* remainp) {
    ctrl = Serialization::decode_i8(ptrp, remainp);
  }
  virtual std::ostream& print(std::ostream& out) const;

  uint8_t ctrl;
};


//
class FieldUpdate_MATH : public FieldUpdate {
  public:
  enum OP : uint8_t {
    EQUAL           = 0x00,
    PLUS            = 0x01,
    MULTIPLY        = 0x02,
    DIVIDE          = 0x03
  };
  SWC_CAN_INLINE
  FieldUpdate_MATH(OP a_op=OP::EQUAL) noexcept : op(a_op) { }
  SWC_CAN_INLINE
  FieldUpdate_MATH(const uint8_t** ptrp, size_t* remainp)
                  : FieldUpdate(ptrp, remainp),
                    op(OP(Serialization::decode_i8(ptrp, remainp))) {
  }
  SWC_CAN_INLINE
  void set_op(const char** ptr, uint32_t* remainp) noexcept{
    set_ctrl(ptr, remainp);
    uint8_t len = 0;
    if(*remainp >= 2) {
      if(Condition::str_eq(*ptr, "+=", 2)) {
        op = OP::PLUS;
        len += 2;
      } else if(Condition::str_eq(*ptr, "*=", 2)) {
        op = OP::MULTIPLY;
        len += 2;
      } else if(Condition::str_eq(*ptr, "/=", 2)) {
        op = OP::DIVIDE;
        len += 2;
      } else if(Condition::str_eq(*ptr, "==", 2)) {
        op = OP::EQUAL;
        len += 2;
      }
    }
    if(!len) {
      op = OP::EQUAL;
      if(*remainp && **ptr == '=')
        ++len;
    }
    *ptr += len;
    *remainp -= len;
  }
  template<typename T>
  SWC_CAN_INLINE
  void apply(const T& in_value, T& value) const {
    switch(op) {
      case OP::EQUAL: {
        value = in_value;
        break;
      }
      case OP::PLUS: {
        value += in_value;
        break;
      }
      case OP::MULTIPLY: {
        value *= in_value;
        break;
      }
      case OP::DIVIDE: {
        if(in_value)
          value /= in_value;
        break;
      }
      default: {
        break;
      }
    }
  }
  template<typename T>
  SWC_CAN_INLINE
  void apply(const Field* infieldp, T& field) const {
    apply(reinterpret_cast<const T*>(infieldp)->value, field.value);
  }
  SWC_CAN_INLINE
  uint24_t encoded_length() const noexcept override {
    return FieldUpdate::encoded_length() + 1;
  }
  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const override {
    FieldUpdate::encode(bufp);
    Serialization::encode_i8(bufp, uint8_t(op));
  }
  SWC_CAN_INLINE
  void decode(const uint8_t** ptrp, size_t* remainp) override {
    FieldUpdate::decode(ptrp, remainp);
    op = OP(Serialization::decode_i8(ptrp, remainp));
  }
  virtual std::ostream& print(std::ostream& out) const override;

  OP op;
};

//
class FieldUpdate_LIST : public FieldUpdate {
  public:
  enum OP : uint8_t {
    REPLACE         = 0x00,
    APPEND          = 0x01,
    PREPEND         = 0x02,
    INSERT          = 0x04,
    OVERWRITE       = 0x05,
    ERASE           = 0x06,
    BY_INDEX        = 0x07
  };
  SWC_CAN_INLINE
  FieldUpdate_LIST(OP a_op=OP::REPLACE, uint24_t a_pos=0)
                   noexcept : op(a_op), pos(a_pos) {
  }
  SWC_CAN_INLINE
  FieldUpdate_LIST(const uint8_t** ptrp, size_t* remainp)
                  : FieldUpdate(ptrp, remainp),
                    op(OP(Serialization::decode_i8(ptrp, remainp))),
                    pos(has_pos()
                          ? Serialization::decode_vi24(ptrp, remainp)
                          : uint24_t(0)) {
  }
  SWC_CAN_INLINE
  bool has_pos() const noexcept {
    return op == OP::INSERT ||
           op == OP::OVERWRITE ||
           op == OP::ERASE;
  }
  SWC_CAN_INLINE
  bool is_op_by_idx() const noexcept {
    return op == OP::BY_INDEX;
  }

  template<bool w_by_idx=false>
  SWC_CAN_INLINE
  void set_op(const char** ptr, uint32_t* remainp, int& err) noexcept {
    set_ctrl(ptr, remainp);
    uint8_t len = 0;
    if(*remainp >= 2) {
      if(Condition::str_eq(*ptr, "+=", 2)) {
        op = OP::APPEND;
        len += 2;
      } else if(Condition::str_eq(*ptr, "=+", 2)) {
        op = OP::PREPEND;
        len += 2;
      } else if(Condition::str_eq(*ptr, "+:", 2)) {
        op = OP::INSERT;
        len += 2;
      } else if(Condition::str_eq(*ptr, "=:", 2)) {
        op = OP::OVERWRITE;
        len += 2;
      } else if(Condition::str_eq(*ptr, "-:", 2)) {
        op = OP::ERASE;
        len += 2;
      } else if(w_by_idx && Condition::str_eq(*ptr, "?:", 2)) {
        op = OP::BY_INDEX;
        len += 2;
      } else if(Condition::str_eq(*ptr, "==", 2)) {
        op = OP::REPLACE;
        len += 2;
      }
    }
    if(!len) {
      op = OP::REPLACE;
      if(*remainp && **ptr == '=')
        ++len;
    }
    *ptr += len;
    *remainp -= len;
    if(has_pos()) {
      errno = 0;
      char** end_at = const_cast<char**>(ptr);
      int64_t v = std::strtoll(*ptr, end_at, 10);
      if(errno) {
        err = errno;
      } else if (v > UINT24_MAX || v < INT24_MIN) {
        err = ERANGE;
      } else {
        pos = v;
        len = *end_at - *ptr;
        *ptr += len;
        *remainp -= len;
      }
    }
  }
  SWC_CAN_INLINE
  void apply(const StaticBuffer& infield, StaticBuffer& field) const {
    switch(op) {
      case OP::REPLACE: {
        field.assign(infield.base, infield.size);
        break;
      }
      case OP::APPEND: {
        StaticBuffer value(field.size + infield.size);
        memcpy(value.base, field.base, field.size);
        memcpy(value.base + field.size, infield.base, infield.size);
        field.set(value);
        break;
      }
      case OP::PREPEND: {
        StaticBuffer value(field.size + infield.size);
        memcpy(value.base, infield.base, infield.size);
        memcpy(value.base + infield.size, field.base, field.size);
        field.set(value);
        break;
      }
      case OP::INSERT: {
        uint32_t p(pos > field.size ? uint32_t(field.size) : uint32_t(pos));
        StaticBuffer value(field.size + infield.size);
        memcpy(value.base, field.base, p);
        memcpy(value.base +p, infield.base, infield.size);
        memcpy(value.base +p + infield.size, field.base +p, field.size -p);
        field.set(value);
        break;
      }
      case OP::OVERWRITE: {
        size_t sz;
        uint32_t p;
        if(pos >= field.size) {
          p = field.size;
          sz = field.size + infield.size;
        } else {
          p = pos;
          sz = infield.size < field.size - p
            ? field.size
            : infield.size + p;
        }
        StaticBuffer value(sz);
        memcpy(value.base, field.base, p);
        memcpy(value.base +p, infield.base, infield.size);
        uint32_t at = p + infield.size;
        if(at < field.size)
          memcpy(value.base + at, field.base + at, field.size - at);
        field.set(value);
        break;
      }
      case OP::ERASE: {
        if(pos < field.size) {
          uint32_t p = pos;
          uint32_t remain = 0;
          if(p + infield.size < field.size)
            remain = field.size - (p + infield.size);
          StaticBuffer value(p + remain);
          memcpy(value.base, field.base, p);
          memcpy(value.base + p, field.base + p + infield.size, remain);
          field.set(value);
        }
        break;
      }
      default:
        break;
    }
  }
  SWC_CAN_INLINE
  void apply(const std::string& in_value, std::string& value) const {
    StaticBuffer buf(
      reinterpret_cast<uint8_t*>(value.data()),
      value.size(),
      false
    );
    apply(
      StaticBuffer(
        const_cast<uint8_t*>(
          reinterpret_cast<const uint8_t*>(in_value.data())
        ),
        in_value.size(),
        false
      ),
      buf
    );
    value.assign(reinterpret_cast<const char*>(buf.base), buf.size);
  }
  template<typename T>
  SWC_CAN_INLINE
  void apply(const Field* infieldp, T& field) const {
    apply(*reinterpret_cast<const T*>(infieldp), field);
  }
  SWC_CAN_INLINE
  virtual uint24_t encoded_length() const noexcept override {
    return FieldUpdate::encoded_length() +
           1 +
           (has_pos() ? Serialization::encoded_length_vi24(pos) : 0);
  }
  SWC_CAN_INLINE
  virtual void encode(uint8_t** bufp) const override {
    FieldUpdate::encode(bufp);
    Serialization::encode_i8(bufp, uint8_t(op));
    if(has_pos())
      Serialization::encode_vi24(bufp, pos);
  }
  SWC_CAN_INLINE
  virtual void decode(const uint8_t** ptrp, size_t* remainp) override {
    FieldUpdate::decode(ptrp, remainp);
    op = OP(Serialization::decode_i8(ptrp, remainp));
    if(has_pos())
      pos = Serialization::decode_vi24(ptrp, remainp);;
  }
  virtual std::ostream& print(std::ostream& out) const override;

  OP       op;
  uint24_t pos;
};


template<typename UpdateField_T>
class FieldUpdate_BY_IDX : public UpdateField_T {
  public:
  SWC_CAN_INLINE
  FieldUpdate_BY_IDX(uint24_t i) noexcept : UpdateField_T(), idx(i) { }
  SWC_CAN_INLINE
  FieldUpdate_BY_IDX(const uint8_t** ptrp, size_t* remainp)
                    : UpdateField_T(ptrp, remainp),
                      idx(Serialization::decode_vi24(ptrp, remainp)) {
  }
  virtual ~FieldUpdate_BY_IDX() noexcept { }
  SWC_CAN_INLINE
  void set_idx(uint24_t i) noexcept {
    idx = i;
  }
  SWC_CAN_INLINE
  uint24_t encoded_length() const noexcept override {
    return UpdateField_T::encoded_length() +
           Serialization::encoded_length_vi24(idx);
  }
  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const override {
    UpdateField_T::encode(bufp);
    Serialization::encode_vi24(bufp, idx);
  }
  SWC_CAN_INLINE
  void decode(const uint8_t** ptrp, size_t* remainp) override {
    UpdateField_T::decode(ptrp, remainp);
    idx = Serialization::decode_vi24(ptrp, remainp);
  }
  virtual std::ostream& print(std::ostream& out) const override {
    return UpdateField_T::print(out << " idx=" << idx << ' ');
  }
  uint24_t idx;
};


//
template<typename UpdateField_T, typename ValueT>
class FieldUpdate_LIST_ITEMS final
    : public FieldUpdate_LIST,
      private Core::Vector<FieldUpdate_BY_IDX<UpdateField_T>> {
  using ItemsT = Core::Vector<FieldUpdate_BY_IDX<UpdateField_T>>;
  using DataT = Core::Vector<ValueT>;
  mutable DataT* data;
  public:
  FieldUpdate_LIST_ITEMS(const FieldUpdate_LIST_ITEMS&)            = delete;
  FieldUpdate_LIST_ITEMS& operator=(const FieldUpdate_LIST_ITEMS&) = delete;
  FieldUpdate_LIST_ITEMS(FieldUpdate_LIST_ITEMS&&)                 = delete;
  FieldUpdate_LIST_ITEMS& operator=(FieldUpdate_LIST_ITEMS&&)      = delete;
  SWC_CAN_INLINE
  FieldUpdate_LIST_ITEMS(OP a_op=OP::REPLACE, uint24_t a_pos=0) noexcept
                        : FieldUpdate_LIST(a_op, a_pos), data(nullptr) {
  }
  SWC_CAN_INLINE
  FieldUpdate_LIST_ITEMS(const uint8_t** ptrp, size_t* remainp)
                        : FieldUpdate_LIST(ptrp, remainp), data(nullptr) {
    if(is_op_by_idx()) {
      uint24_t len = Serialization::decode_vi24(ptrp, remainp);
      ItemsT::reserve(len);
      for(;len; --len)
        ItemsT::emplace_back(ptrp, remainp);
    }
  }
  virtual ~FieldUpdate_LIST_ITEMS() noexcept {
    delete data;
  }
  SWC_CAN_INLINE
  UpdateField_T& add_item(uint24_t idx) {
    return ItemsT::emplace_back(idx);
  }
  template<typename T>
  SWC_CAN_INLINE
  void apply(const Field* infieldp, T& field) const {
    auto infield(reinterpret_cast<const T*>(infieldp));
    switch(op) {
      case OP::REPLACE:
      case OP::APPEND:
      case OP::PREPEND: {
        FieldUpdate_LIST::apply(infieldp, field);
        break;
      }
      case OP::INSERT: {
        if(!data)
          infield->convert_to(*(data = new DataT()));
        const DataT& vec_in = *data;
        DataT vec;
        field.convert_to(vec);
        vec.insert(
          pos > vec.size() ? vec.size() : uint32_t(pos),
          vec_in.cbegin(),
          vec_in.cend()
        );
        field.set_from(vec);
        break;
      }
      case OP::OVERWRITE: {
        if(!data)
          infield->convert_to(*(data = new DataT()));
        const DataT& vec_in = *data;
        DataT vec;
        field.convert_to(vec);
        size_t sz;
        uint32_t p;
        if(pos >= vec.size()) {
          p = vec.size();
          sz = vec.size() + vec_in.size();
        } else {
          p = pos;
          sz = vec_in.size() < vec.size() - p
            ? vec.size()
            : vec_in.size() + p;
        }
        DataT items;
        items.reserve(sz);
        for(uint32_t i = 0; i < p; ++i)
          items.push_back(std::move(vec[i]));
        for(auto it = vec_in.cbegin(); it < vec_in.cend(); ++it)
          items.push_back(*it);
        if(vec.size() > items.size()) {
          for(auto it = vec.begin() + items.size(); it < vec.end(); ++it)
            items.push_back(std::move(*it));
        }
        field.set_from(items);
        break;
      }
      case OP::ERASE: {
        if(field.size) {
          if(!data)
            infield->convert_to(*(data = new DataT()));
          DataT vec;
          auto has_count = field.convert_less_to(vec, pos, data->size());
          if(has_count != vec.size())
            field.set_from(vec);
        }
        break;
      }
      case OP::BY_INDEX: {
        if(!data)
          infield->convert_to(*(data = new DataT()));
        const DataT& vec_in = *data;
        DataT vec;
        field.convert_to(vec);
        auto iti = ItemsT::cbegin();
        auto itu = vec_in.cbegin();
        for( ;iti < ItemsT::cend(); ++iti, ++itu) {
          if(iti->idx < vec.size())
            iti->apply(*itu, vec[iti->idx]);
          else if(!iti->is_no_add_field())
            iti->apply(*itu, vec.emplace_back());
        }
        field.set_from(vec);
        break;
      }
      default:
        break;
    }
  }
  SWC_CAN_INLINE
  uint24_t encoded_length() const noexcept override {
    uint24_t len = FieldUpdate_LIST::encoded_length();
    if(is_op_by_idx()) {
      len += Serialization::encoded_length_vi24(ItemsT::size());
      for(auto& item : *this)
        len += item.encoded_length();
    }
    return len;
  }
  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const override {
    FieldUpdate_LIST::encode(bufp);
    if(is_op_by_idx()) {
      Serialization::encode_vi24(bufp, ItemsT::size());
      for(auto& item : *this)
        item.encode(bufp);
    }
  }
  SWC_CAN_INLINE
  void decode(const uint8_t** ptrp, size_t* remainp) override {
    FieldUpdate_LIST::decode(ptrp, remainp);
    if(is_op_by_idx()) {
      uint24_t len = Serialization::decode_vi24(ptrp, remainp);
      ItemsT::reserve(len);
      for(;len; --len)
        ItemsT::emplace_back(ptrp, remainp);
    }
  }
  virtual std::ostream& print(std::ostream& out) const override {
    FieldUpdate_LIST::print(out);
    if(is_op_by_idx()) {
      out << " items=" << ItemsT::size() << '[';
      for(auto& item : *this)
        item.print(out) << ',';
      out << ']';
    }
    return out;
  }

};

typedef FieldUpdate_LIST_ITEMS<FieldUpdate_MATH, int64_t>
  FieldUpdate_LIST_INT64;
typedef FieldUpdate_LIST_ITEMS<FieldUpdate_LIST, std::string>
  FieldUpdate_LIST_BYTES;
//



}}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellValueSerialFieldUpdate.cc"
#endif

#endif // swcdb_db_cells_CellValueSerialFieldUpdate_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellValueSerialFieldUpdate_h
#define swcdb_db_cells_CellValueSerialFieldUpdate_h

#include "swcdb/db/Cells/CellValueSerialField.h"
#include <set>


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
  static constexpr const uint8_t CTRL_VALUE_SET     = 0x04;
  static constexpr const uint8_t CTRL_VALUE_DEL     = 0x08;
  SWC_CAN_INLINE
  FieldUpdate() noexcept : ctrl(CTRL_DEFAULT) { }
  SWC_CAN_INLINE
  FieldUpdate(const uint8_t** ptrp, size_t* remainp)
              : ctrl(Serialization::decode_i8(ptrp, remainp)) {
  }
  virtual ~FieldUpdate() noexcept { }
  SWC_CAN_INLINE
  bool set_ctrl(const char** ptr, uint32_t* remainp,
                bool w_value_ctrl) noexcept {
    if(*remainp && **ptr == '!') {
      set_no_add_field();
      ++*ptr;
      --*remainp;
    }
    if(w_value_ctrl && *remainp) {
      if(**ptr == 'I') {
        set_value_set();
        ++*ptr;
        --*remainp;
        return false;
      } else if(**ptr == 'O') {
        set_value_del();
        ++*ptr;
        --*remainp;
        return false;
      }
    }
    if(*remainp >= 3 && Condition::str_eq(*ptr, "DEL", 3)) {
      set_delete_field();
      *ptr += 3;
      *remainp -=3;
      return false;
    }
    return true;
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
  SWC_CAN_INLINE
  bool is_delete_field() const noexcept {
    return ctrl & CTRL_DELETE_FIELD;
  }
  SWC_CAN_INLINE
  void set_value_set() noexcept {
    ctrl |= CTRL_VALUE_SET;
  }
  SWC_CAN_INLINE
  bool is_value_set() const noexcept {
    return ctrl & CTRL_VALUE_SET;
  }
  SWC_CAN_INLINE
  void set_value_del() noexcept {
    ctrl |= CTRL_VALUE_DEL;
  }
  SWC_CAN_INLINE
  bool is_value_del() const noexcept {
    return ctrl & CTRL_VALUE_DEL;
  }
  SWC_CAN_INLINE
  bool is_sub_op_required() const noexcept {
    return !(is_value_del() || is_value_set() || is_delete_field());
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
                    op(is_sub_op_required()
                        ? OP(Serialization::decode_i8(ptrp, remainp))
                        : OP::EQUAL
                      ) {
  }
  SWC_CAN_INLINE
  void set_op(const char** ptr, uint32_t* remainp,
              bool w_value_ctrl=false) noexcept {
    if(!set_ctrl(ptr, remainp, w_value_ctrl))
      return;
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
  template<typename T>
  SWC_CAN_INLINE
  bool is_matching(Condition::Comp comp,
                   const T& p1, const T& p2) const noexcept {
    return Condition::is_matching(comp, p1, p2);
  }
  SWC_CAN_INLINE
  uint24_t encoded_length() const noexcept override {
    return FieldUpdate::encoded_length() + is_sub_op_required();
  }
  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const override {
    FieldUpdate::encode(bufp);
    if(is_sub_op_required())
      Serialization::encode_i8(bufp, uint8_t(op));
  }
  SWC_CAN_INLINE
  void decode(const uint8_t** ptrp, size_t* remainp) override {
    FieldUpdate::decode(ptrp, remainp);
    if(is_sub_op_required())
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
    INSERT          = 0x03,
    OVERWRITE       = 0x04,
    ERASE           = 0x05,
    BY_UNIQUE       = 0x06,
    BY_COND         = 0x07,
    BY_INDEX        = 0x08
  };
  SWC_CAN_INLINE
  FieldUpdate_LIST(OP a_op=OP::REPLACE, uint24_t a_pos=0)
                   noexcept : op(a_op), pos(a_pos) {
  }
  SWC_CAN_INLINE
  FieldUpdate_LIST(const uint8_t** ptrp, size_t* remainp)
                  : FieldUpdate(ptrp, remainp),
                    op(is_sub_op_required()
                        ? OP(Serialization::decode_i8(ptrp, remainp))
                        : OP::REPLACE
                      ),
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
  bool is_op_by_unique() const noexcept {
    return op == OP::BY_UNIQUE;
  }
  SWC_CAN_INLINE
  bool is_op_by_cond() const noexcept {
    return op == OP::BY_COND;
  }
  SWC_CAN_INLINE
  bool is_op_by_idx() const noexcept {
    return op == OP::BY_INDEX;
  }
  SWC_CAN_INLINE
  bool is_op_require_data() const noexcept {
    return is_op_by_idx() || is_op_by_cond();
  }
  SWC_CAN_INLINE
  bool has_item_op() const noexcept {
    return is_op_by_idx() || is_op_by_unique() || is_op_by_cond();
  }
  template<bool is_upper_type=false>
  SWC_CAN_INLINE
  void set_op(const char** ptr, uint32_t* remainp, int& err,
              bool w_value_ctrl=false) noexcept {
    if(!set_ctrl(ptr, remainp, w_value_ctrl))
      return;
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
      } else if(is_upper_type && Condition::str_eq(*ptr, "U~", 2)) {
        op = OP::BY_UNIQUE;
        len += 2;
      } else if(is_upper_type && Condition::str_eq(*ptr, "C~", 2)) {
        op = OP::BY_COND;
        len += 2;
      } else if(is_upper_type && Condition::str_eq(*ptr, "?:", 2)) {
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
  template<typename T>
  SWC_CAN_INLINE
  bool is_matching(Condition::Comp comp,
                   const T& p1, const T& p2) const noexcept {
    return Condition::is_matching_extended(
      comp,
      reinterpret_cast<const uint8_t*>(p1.data()),
      p1.size(),
      reinterpret_cast<const uint8_t*>(p2.data()),
      p2.size()
    );
  }
  SWC_CAN_INLINE
  virtual uint24_t encoded_length() const noexcept override {
    return FieldUpdate::encoded_length() +
           is_sub_op_required() +
           (has_pos() ? Serialization::encoded_length_vi24(pos) : 0);
  }
  SWC_CAN_INLINE
  virtual void encode(uint8_t** bufp) const override {
    FieldUpdate::encode(bufp);
    if(is_sub_op_required()) {
      Serialization::encode_i8(bufp, uint8_t(op));
      if(has_pos())
        Serialization::encode_vi24(bufp, pos);
    }
  }
  SWC_CAN_INLINE
  virtual void decode(const uint8_t** ptrp, size_t* remainp) override {
    FieldUpdate::decode(ptrp, remainp);
    if(is_sub_op_required()) {
      op = OP(Serialization::decode_i8(ptrp, remainp));
      if(has_pos())
        pos = Serialization::decode_vi24(ptrp, remainp);;
    }
  }
  virtual std::ostream& print(std::ostream& out) const override;

  OP       op;
  uint24_t pos;
};


template<typename UpdateField_T>
class FieldUpdate_Ext final : public UpdateField_T {
  public:
  SWC_CAN_INLINE
  FieldUpdate_Ext(uint32_t i) noexcept : UpdateField_T(), data(i) { }
  SWC_CAN_INLINE
  FieldUpdate_Ext(bool w_data, const uint8_t** ptrp, size_t* remainp)
                  : UpdateField_T(ptrp, remainp),
                    data(w_data
                          ? Serialization::decode_vi32(ptrp, remainp)
                          : uint32_t(0)
                        ) {
  }
  virtual ~FieldUpdate_Ext() noexcept { }
  SWC_CAN_INLINE
  void set_data(uint32_t i) noexcept {
    data = i;
  }
  SWC_CAN_INLINE
  uint32_t ext_encoded_length(bool w_data) const noexcept {
    return UpdateField_T::encoded_length() +
           (w_data
            ? Serialization::encoded_length_vi32(data)
            : 0);
  }
  SWC_CAN_INLINE
  void ext_encode(bool w_data, uint8_t** bufp) const {
    UpdateField_T::encode(bufp);
    if(w_data)
      Serialization::encode_vi32(bufp, data);
  }
  SWC_CAN_INLINE
  void ext_decode(bool w_data, const uint8_t** ptrp, size_t* remainp) {
    UpdateField_T::decode(ptrp, remainp);
    if(w_data)
      data = Serialization::decode_vi32(ptrp, remainp);
  }
  std::ostream& ext_print(bool w_data, std::ostream& out) const {
    if(w_data)
      out << " data=" << data;
    return UpdateField_T::print(out << ' ');
  }
  uint32_t data;
};


//
template<typename UpdateField_T, typename ValueT>
class FieldUpdate_LIST_ITEMS final
    : public FieldUpdate_LIST,
      private Core::Vector<FieldUpdate_Ext<UpdateField_T>> {
  using ItemsT = Core::Vector<FieldUpdate_Ext<UpdateField_T>>;
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
    if(has_item_op()) {
      uint24_t len = Serialization::decode_vi24(ptrp, remainp);
      ItemsT::reserve(len);
      bool w_data = is_op_require_data();
      for(;len; --len)
        ItemsT::emplace_back(w_data, ptrp, remainp);
    }
  }
  virtual ~FieldUpdate_LIST_ITEMS() noexcept {
    delete data;
  }
  SWC_CAN_INLINE
  UpdateField_T& add_item(uint32_t i) {
    return ItemsT::emplace_back(i);
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
      case OP::BY_UNIQUE: {
        if(!data)
          infield->convert_to(*(data = new DataT()));
        const DataT& vec_in = *data;
        std::set<ValueT> vec;
        field.convert_to(vec);
        auto itu = vec_in.cbegin();
        for(auto iti = ItemsT::cbegin() ;iti < ItemsT::cend(); ++iti, ++itu) {
          if(iti->is_value_set())
            vec.insert(*itu);
          else if(iti->is_value_del())
            vec.erase(*itu);
        }
        field.set_from(vec);
        break;
      }
      case OP::BY_COND: {
        if(!data)
          infield->convert_to(*(data = new DataT()));
        const DataT& vec_in = *data;
        DataT vec;
        field.convert_to(vec);
        auto itu = vec_in.cbegin();
        for(auto iti = ItemsT::cbegin() ;iti < ItemsT::cend(); ++iti, ++itu) {
          auto it = vec.cbegin();
          auto comp(Condition::Comp(iti->data));
          for(; it < vec.cend(); ++it) {
            if(iti->is_matching(comp, *itu, *it))
              break;
          }
          if(it == vec.cend()) {
            if(iti->is_value_set())
              vec.emplace_back(*itu);
          } else if(iti->is_value_del()) {
            vec.erase(it);
            --iti;
            --itu;
          }
        }
        field.set_from(vec);
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
          if(iti->data < vec.size())
            iti->apply(*itu, vec[iti->data]);
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
    if(has_item_op()) {
      len += Serialization::encoded_length_vi24(ItemsT::size());
      bool w_data = is_op_require_data();
      for(auto& item : *this)
        len += item.ext_encoded_length(w_data);
    }
    return len;
  }
  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const override {
    FieldUpdate_LIST::encode(bufp);
    if(has_item_op()) {
      Serialization::encode_vi24(bufp, ItemsT::size());
      bool w_data = is_op_require_data();
      for(auto& item : *this)
        item.ext_encode(w_data, bufp);
    }
  }
  SWC_CAN_INLINE
  void decode(const uint8_t** ptrp, size_t* remainp) override {
    FieldUpdate_LIST::decode(ptrp, remainp);
    if(has_item_op()) {
      uint24_t len = Serialization::decode_vi24(ptrp, remainp);
      ItemsT::reserve(len);
      bool w_data = is_op_require_data();
      for(;len; --len)
        ItemsT::emplace_back(w_data, ptrp, remainp);
    }
  }
  virtual std::ostream& print(std::ostream& out) const override {
    FieldUpdate_LIST::print(out);
    if(has_item_op()) {
      out << " items=" << ItemsT::size() << '[';
      bool w_data = is_op_require_data();
      for(auto& item : *this)
        item.ext_print(w_data, out) << ',';
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

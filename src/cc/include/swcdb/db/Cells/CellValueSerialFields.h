/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellValueSerialFields_h
#define swcdb_db_cells_CellValueSerialFields_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Cell {



//! The SWC-DB Serial Cell C++ namespace 'SWC::DB::Cell::Serial'
namespace Serial {



//! The SWC-DB Serial Value Cell C++ namespace 'SWC::DB::Cell::Serial::Value'
namespace Value {



/* SERIALIZATION:
    Field[(i8 type, vi24 fid, FieldEnc(dep.type)), .. ]
*/

enum Type : uint8_t {
  UNKNOWN       = 0x00,
  INT64         = 0x01,
  DOUBLE        = 0x02,
  BYTES         = 0x03,
  KEY           = 0x04,
  LIST_INT64    = 0x05,
  LIST_BYTES    = 0x06,

  //LIST_DOUBLE   = 0x06,
  //LIST_ANY      = 0x08,
  //MAP     = 0x06,

};

const char* SWC_CONST_FUNC to_string(Type typ) noexcept;

SWC_CAN_INLINE
Type read_type(const uint8_t** bufp, size_t* remainp) {
  return Type(Serialization::decode_i8(bufp, remainp));
}

SWC_CAN_INLINE
uint24_t read_field_id(const uint8_t** bufp, size_t* remainp) {
  return Serialization::decode_vi24(bufp, remainp);
}

SWC_CAN_INLINE
void skip_type_and_id(const uint8_t** bufp, size_t* remainp) {
  read_type(bufp, remainp);
  read_field_id(bufp, remainp);
}
//



// Field-Base
struct Field {

  uint24_t fid;

  SWC_CAN_INLINE
  Field() noexcept { }

  constexpr SWC_CAN_INLINE
  Field(uint24_t a_fid) noexcept : fid(a_fid) { }

  SWC_CAN_INLINE
  Field(const uint8_t** bufp, size_t* remainp)
        : fid(Serialization::decode_vi24(bufp, remainp)) {
  }

  virtual ~Field() noexcept { }

  virtual Type type() const noexcept = 0;

  virtual size_t encoded_length() const noexcept {
    return 1 + Serialization::encoded_length_vi24(fid);
  }

  virtual void encode(uint8_t** bufp) const = 0;

  SWC_CAN_INLINE
  void encode(uint8_t** bufp, Type type) const {
    Serialization::encode_i8(bufp, type);
    Serialization::encode_vi24(bufp, fid);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    fid = Serialization::decode_vi24(bufp, remainp);
  }

  virtual void print(std::ostream& out) const = 0;

};
//



// Field INT64
struct Field_INT64 : Field {

  int64_t value;

  SWC_CAN_INLINE
  Field_INT64() noexcept { }

  constexpr SWC_CAN_INLINE
  Field_INT64(uint24_t a_fid, int64_t a_value) noexcept
              : Field(a_fid), value(a_value) { }

  Field_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_INT64() noexcept { }

  Type type() const noexcept override { return Type::INT64; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void print(std::ostream& out) const override;

};
//



// Field DOUBLE
struct Field_DOUBLE : Field {

  long double value;

  SWC_CAN_INLINE
  Field_DOUBLE() noexcept { }

  constexpr SWC_CAN_INLINE
  Field_DOUBLE(uint24_t a_fid, const long double& a_value) noexcept
              : Field(a_fid), value(a_value) { }

  Field_DOUBLE(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_DOUBLE() noexcept { }

  Type type() const noexcept override { return Type::DOUBLE; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void print(std::ostream& out) const override;

};
//



// Field BYTES
struct Field_BYTES : Field, StaticBuffer {

  SWC_CAN_INLINE
  Field_BYTES() noexcept { }

  Field_BYTES(uint24_t fid, const uint8_t* data, uint32_t len,
              bool take_ownership=false);

  Field_BYTES(const uint8_t** bufp, size_t* remainp,
              bool take_ownership=false);

  virtual ~Field_BYTES() noexcept { }

  Type type() const noexcept override { return Type::BYTES; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  template<typename T>
  void convert_to(T& item) const {
    if(size) {
      item.assign(
        reinterpret_cast<const typename T::value_type*>(base),
        size
      );
    }
  }

  void print(std::ostream& out) const override;

};
//



// Field KEY
struct Field_KEY : Field {

  SWC_CAN_INLINE
  Field_KEY() noexcept { }

  Field_KEY(uint24_t fid, const Key& key, bool take_ownership=false);

  Field_KEY(const uint8_t** bufp, size_t* remainp, bool take_ownership=false);

  virtual ~Field_KEY() noexcept { }

  Type type() const noexcept override { return Type::KEY; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void decode(const uint8_t** bufp, size_t* remainp,
              bool take_ownership=false);

  void print(std::ostream& out) const override;

  Key key;

};
//



// Field LIST_INT64
struct Field_LIST_INT64 : Field, StaticBuffer {

  SWC_CAN_INLINE
  Field_LIST_INT64() noexcept { }

  template<typename T>
  Field_LIST_INT64(uint24_t a_fid, const T& items) : Field(a_fid) {
    uint32_t len = 0;
    for(auto& v : items)
      len += Serialization::encoded_length_vi64(v);
    reallocate(len);
    uint8_t* ptr = base;
    for(auto& v : items)
      Serialization::encode_vi64(&ptr, v);
  }

  Field_LIST_INT64(const uint8_t** bufp, size_t* remainp,
                   bool take_ownership=false);

  virtual ~Field_LIST_INT64() noexcept { }

  Type type() const noexcept override { return Type::LIST_INT64; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  template<typename T>
  void convert_to(T& items) const {
    if(size) {
      const uint8_t* ptr = base;
      for(size_t remain = size; remain;)
        items.push_back(Serialization::decode_vi64(&ptr, &remain));
    }
  }

  void print(std::ostream& out) const override;

};
//



// Field LIST_BYTES
struct Field_LIST_BYTES : Field, StaticBuffer {

  SWC_CAN_INLINE
  Field_LIST_BYTES() noexcept { }

  template<typename T>
  Field_LIST_BYTES(uint24_t a_fid, const T& items) : Field(a_fid) {
    uint32_t len = 0;
    for(auto& v : items)
      len += Serialization::encoded_length_bytes(v.size());
    reallocate(len);
    uint8_t* ptr = base;
    for(auto& v : items)
      Serialization::encode_bytes(&ptr, v.data(), v.size());
  }

  Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp,
                   bool take_ownership=false);

  virtual ~Field_LIST_BYTES() noexcept { }

  Type type() const noexcept override { return Type::LIST_BYTES; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  template<typename T>
  void convert_to(T& items) const {
    if(size) {
      const uint8_t* ptr = base;
      for(size_t remain = size; remain;) {
        size_t len;
        const char* cptr = reinterpret_cast<
          const typename T::value_type::value_type*>(
            Serialization::decode_bytes(&ptr, &remain, &len));
        items.emplace_back(cptr, len);
      }
    }
  }

  void print(std::ostream& out) const override;

};
//


/// Field for Update
class FieldUpdate {
  public:
  SWC_CAN_INLINE
  FieldUpdate() noexcept { }
  virtual ~FieldUpdate() noexcept { }
  virtual bool without_adding_field() const noexcept         = 0;
  virtual uint24_t encoded_length() const noexcept           = 0;
  virtual void encode(uint8_t** bufp) const                  = 0;
  virtual void decode(const uint8_t** ptrp, size_t* remainp) = 0;
  virtual std::ostream& print(std::ostream& out) const       = 0;
};


/*
FieldsWriter wfields;
wfields.add(123);

cell.set_value(wfields.base, wfields.fill(), true);
or
cell.set_value(encoder, wfields.base, wfields.fill());
*/
struct FieldsWriter final : DynamicBuffer {

  constexpr SWC_CAN_INLINE
  FieldsWriter() noexcept : index_count(0) { }

  ~FieldsWriter() noexcept { }

  SWC_CAN_INLINE
  void add(Field* field) {
    ensure(field->encoded_length());
    field->encode(&ptr);
  }

  SWC_CAN_INLINE
  void add(FieldUpdate* ufield) {
    ensure(ufield->encoded_length());
    ufield->encode(&ptr);
  }

  void add(uint24_t fid, const int64_t& value);

  void add(uint24_t fid, const long double& value);

  void add(uint24_t fid, const uint8_t* data, uint32_t len);

  void add(uint24_t fid, const Key& key);

  void add(uint24_t fid, const Core::Vector<int64_t>& items);

  void add(uint24_t fid, const std::vector<int64_t>& items);

  void add(uint24_t fid, const Core::Vector<std::string>& items);

  void add(uint24_t fid, const std::vector<std::string>& items);


  SWC_CAN_INLINE
  void add(const int64_t& value) {
    add(index_count++, value);
  }

  SWC_CAN_INLINE
  void add(const long double& value) {
    add(index_count++, value);
  }

  SWC_CAN_INLINE
  void add(const uint8_t* data, uint32_t len) {
    add(index_count++, data, len);
  }

  SWC_CAN_INLINE
  void add(const std::string& data) {
    add(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
  }

  SWC_CAN_INLINE
  void add(uint24_t fid, const std::string& data) {
    add(fid, reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
  }

  SWC_CAN_INLINE
  void add(const char* data, uint32_t len) {
    add(reinterpret_cast<const uint8_t*>(data), len);
  }

  SWC_CAN_INLINE
  void add(uint24_t fid, const char* data, uint32_t len) {
    add(fid, reinterpret_cast<const uint8_t*>(data), len);
  }

  SWC_CAN_INLINE
  void add(const Key& key) {
    add(index_count++, key);
  }

  SWC_CAN_INLINE
  void add(const Core::Vector<int64_t>& items) {
    add(index_count++, items);
  }

  SWC_CAN_INLINE
  void add(const std::vector<int64_t>& items) {
    add(index_count++, items);
  }

  SWC_CAN_INLINE
  void add(const Core::Vector<std::string>& items) {
    add(index_count++, items);
  }

  SWC_CAN_INLINE
  void add(const std::vector<std::string>& items) {
    add(index_count++, items);
  }

  void print(std::ostream& out) const;

  uint24_t index_count;

};
//



//
struct Fields {

  static void print(const uint8_t* ptr, size_t remain, std::ostream& out);

  static void display(const uint8_t* ptr, size_t remain, std::ostream& out);

};
//





//
class FieldUpdate_MATH : public FieldUpdate {
  public:
  static constexpr const uint8_t OP_EQUAL           = 0x00;
  static constexpr const uint8_t OP_PLUS            = 0x01;
  static constexpr const uint8_t OP_MULTIPLY        = 0x02;
  static constexpr const uint8_t OP_DIVIDE          = 0x04;
  static constexpr const uint8_t CTRL_NO_ADD_FIELD  = 0xF0;
  SWC_CAN_INLINE
  FieldUpdate_MATH(uint8_t a_op=OP_EQUAL) noexcept : op(a_op) { }
  SWC_CAN_INLINE
  FieldUpdate_MATH(const uint8_t** ptrp, size_t* remainp)
                  : op(Serialization::decode_i8(ptrp, remainp)) {
  }
  SWC_CAN_INLINE
  bool without_adding_field() const noexcept override {
    return op & CTRL_NO_ADD_FIELD;
  }
  SWC_CAN_INLINE
  void set_op(const char** ptr, uint32_t* remainp) noexcept{
    op = OP_EQUAL;
    if(**ptr == '!') {
      op |= CTRL_NO_ADD_FIELD;
      ++*ptr;
      --*remainp;
    }
    uint8_t len = 0;
    if(*remainp >= 2) {
      if(Condition::str_eq(*ptr, "+=", 2)) {
        op |= OP_PLUS;
        len += 2;
      } else if(Condition::str_eq(*ptr, "*=", 2)) {
        op |= OP_MULTIPLY;
        len += 2;
      } else if(Condition::str_eq(*ptr, "/=", 2)) {
        op |= OP_DIVIDE;
        len += 2;
      } else if(Condition::str_eq(*ptr, "==", 2)) {
        len += 2;
      }
    }
    if(!len && *remainp && **ptr == '=') {
      ++len;
    }
    *ptr += len;
    *remainp -= len;
  }
  template<typename T>
  SWC_CAN_INLINE
  void apply(const Field* infieldp, T& field) const {
    const T* infield(reinterpret_cast<const T*>(infieldp));
    switch(op | CTRL_NO_ADD_FIELD) {
      case OP_EQUAL | CTRL_NO_ADD_FIELD: {
        field.value = infield->value;
        break;
      }
      case OP_PLUS | CTRL_NO_ADD_FIELD: {
        field.value += infield->value;
        break;
      }
      case OP_MULTIPLY | CTRL_NO_ADD_FIELD: {
        field.value *= infield->value;
        break;
      }
      case OP_DIVIDE | CTRL_NO_ADD_FIELD: {
        if(infield->value)
          field.value /= infield->value;
        break;
      }
      default: {
        break;
      }
    }
  }
  SWC_CAN_INLINE
  uint24_t encoded_length() const noexcept override {
    return 1;
  }
  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const override {
    Serialization::encode_i8(bufp, op);
  }
  SWC_CAN_INLINE
  void decode(const uint8_t** ptrp, size_t* remainp) override {
    op = Serialization::decode_i8(ptrp, remainp);
  }
  std::ostream& print(std::ostream& out) const override {
    uint8_t tmp = op;
    if(without_adding_field()) {
      tmp ^= CTRL_NO_ADD_FIELD;
      out << "CTRL_NO_ADD_FIELD ";
    }
    switch(tmp) {
      case OP_EQUAL:
        return out << "EQUAL";
      case OP_PLUS:
        return out << "PLUS";
      case OP_MULTIPLY:
        return out << "MULTIPLY";
      case OP_DIVIDE:
        return out << "DIVIDE";
      default:
        return out << "UNKNOWN";
    }
  }
  uint8_t op;
};

//
class FieldUpdate_LIST : public FieldUpdate {
  public:
  static constexpr const uint8_t OP_REPLACE         = 0x00;
  static constexpr const uint8_t OP_APPEND          = 0x01;
  static constexpr const uint8_t OP_PREPEND         = 0x02;
  static constexpr const uint8_t OP_INSERT          = 0x04;
  static constexpr const uint8_t OP_OVERWRITE       = 0x08;
  static constexpr const uint8_t CTRL_NO_ADD_FIELD  = 0xF0;
  SWC_CAN_INLINE
  FieldUpdate_LIST(uint8_t a_op=OP_REPLACE, uint24_t a_pos=0)
                   noexcept : op(a_op), pos(a_pos) {
  }
  SWC_CAN_INLINE
  FieldUpdate_LIST(const uint8_t** ptrp, size_t* remainp)
                  : op(Serialization::decode_i8(ptrp, remainp)),
                    pos(has_pos()
                          ? Serialization::decode_vi24(ptrp, remainp)
                          : uint24_t(0)) {
  }
  SWC_CAN_INLINE
  virtual bool without_adding_field() const noexcept override {
    return op & CTRL_NO_ADD_FIELD;
  }
  SWC_CAN_INLINE
  bool has_pos() const noexcept {
    return op & OP_INSERT || op & OP_OVERWRITE;
  }
  SWC_CAN_INLINE
  void set_op(const char** ptr, uint32_t* remainp, int& err) noexcept{
    op = OP_REPLACE;
    if(**ptr == '!') {
      op |= CTRL_NO_ADD_FIELD;
      ++*ptr;
      --*remainp;
    }
    uint8_t len = 0;
    if(*remainp >= 2) {
      if(Condition::str_eq(*ptr, "+=", 2)) {
        op |= OP_APPEND;
        len += 2;
      } else if(Condition::str_eq(*ptr, "=+", 2)) {
        op |= OP_PREPEND;
        len += 2;
      } else if(Condition::str_eq(*ptr, "+:", 2)) {
        op |= OP_INSERT;
        len += 2;
      } else if(Condition::str_eq(*ptr, "=:", 2)) {
        op |= OP_OVERWRITE;
        len += 2;
      } else if(Condition::str_eq(*ptr, "==", 2)) {
        len += 2;
      }
    }
    if(!len && *remainp && **ptr == '=') {
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
  virtual uint24_t encoded_length() const noexcept override {
    return 1 + (has_pos() ? Serialization::encoded_length_vi24(pos) : 0);
  }
  SWC_CAN_INLINE
  void apply(const Field* infieldp, Field_BYTES& field) const {
    const Field_BYTES* infield(
      reinterpret_cast<const Field_BYTES*>(infieldp));
    switch(op | CTRL_NO_ADD_FIELD) {
      case OP_REPLACE | CTRL_NO_ADD_FIELD: {
        field.assign(infield->base, infield->size);
        break;
      }
      case OP_APPEND | CTRL_NO_ADD_FIELD: {
        StaticBuffer value(field.size + infield->size);
        memcpy(value.base, field.base, field.size);
        memcpy(value.base + field.size, infield->base, infield->size);
        field.set(value);
        break;
      }
      case OP_PREPEND | CTRL_NO_ADD_FIELD: {
        StaticBuffer value(field.size + infield->size);
        memcpy(value.base, infield->base, infield->size);
        memcpy(value.base + infield->size, field.base, field.size);
        field.set(value);
        break;
      }
      case OP_INSERT | CTRL_NO_ADD_FIELD: {
        uint32_t p(pos > field.size ? uint32_t(field.size) : uint32_t(pos));
        StaticBuffer value(field.size + infield->size);
        memcpy(value.base, field.base, p);
        memcpy(value.base +p, infield->base, infield->size);
        memcpy(value.base +p + infield->size, field.base +p, field.size -p);
        field.set(value);
        break;
      }
      case OP_OVERWRITE | CTRL_NO_ADD_FIELD: {
        size_t sz;
        uint32_t p;
        if(pos >= field.size) {
          p = field.size;
          sz = field.size + infield->size;
        } else {
          p = pos;
          sz = infield->size < field.size - p
            ? field.size
            : infield->size + p;
        }
        StaticBuffer value(sz);
        memcpy(value.base, field.base, p);
        memcpy(value.base +p, infield->base, infield->size);
        uint32_t at = p + infield->size;
        if(at < field.size)
          memcpy(value.base + at, field.base + at, field.size - at);
        field.set(value);
        break;
      }
      default:
        break;
    }
  }
  SWC_CAN_INLINE
  virtual void encode(uint8_t** bufp) const override {
    Serialization::encode_i8(bufp, op);
    if(has_pos())
      Serialization::encode_vi24(bufp, pos);
  }
  SWC_CAN_INLINE
  virtual void decode(const uint8_t** ptrp, size_t* remainp) override {
    op = Serialization::decode_i8(ptrp, remainp);
    if(has_pos())
      pos = Serialization::decode_vi24(ptrp, remainp);;
  }
  virtual std::ostream& print(std::ostream& out) const override {
    uint8_t tmp = op;
    if(without_adding_field()) {
      tmp ^= CTRL_NO_ADD_FIELD;
      out << "CTRL_NO_ADD_FIELD ";
    }
    switch(tmp) {
      case OP_REPLACE:
        return out << "REPLACE";
      case OP_APPEND:
        return out << "APPEND";
      case OP_PREPEND:
        return out << "PREPEND";
      case OP_INSERT:
        return out << "INSERT:" << pos;
      case OP_OVERWRITE:
        return out << "OVERWRITE:" << pos;
      default:
        return out << "UNKNOWN";
    }
  }
  uint8_t  op;
  uint24_t pos;
};



/// Fields Reader with FieldsWriter with FieldUpdate serialization
struct FieldUpdateOP {
  Field*        field;
  FieldUpdate*  ufield;
  SWC_CAN_INLINE
  FieldUpdateOP() noexcept : field(nullptr), ufield(nullptr) { }
  SWC_CAN_INLINE
  FieldUpdateOP(FieldUpdateOP&& other) noexcept
                : field(other.field), ufield(other.ufield) {
    other.field = nullptr;
    other.ufield = nullptr;
  }
  SWC_CAN_INLINE
  FieldUpdateOP& operator=(FieldUpdateOP&& other) noexcept {
    delete field;
    delete ufield;
    field = nullptr;
    ufield = nullptr;
    std::swap(field, other.field);
    std::swap(ufield, other.ufield);
    return *this;
  }
  FieldUpdateOP(const FieldUpdateOP&)            = delete;
  FieldUpdateOP& operator=(const FieldUpdateOP&) = delete;
  ~FieldUpdateOP() noexcept {
    delete field;
    delete ufield;
  }
  operator bool() const noexcept {
    return ufield;
  }
};
typedef Core::Vector<const FieldUpdateOP*> FieldUpdateOpPtrs;



struct FieldsUpdaterMap final
    : private std::array<std::map<uint32_t, FieldUpdateOP>*, 7> {
  public:

  uint24_t count;

  SWC_CAN_INLINE
  FieldsUpdaterMap() noexcept : count(0) {
    fill(nullptr);
  }

  SWC_CAN_INLINE
  FieldsUpdaterMap(const uint8_t* ptr, size_t remain, bool take_ownership)
                  : count(0) {
    fill(nullptr);
    decode(ptr, remain, take_ownership);
  }

  SWC_CAN_INLINE
  ~FieldsUpdaterMap() noexcept {
    for(auto it_t = cbegin() + 1; it_t != cend(); ++it_t) {
      delete *it_t;
    }
  }

  void decode(const uint8_t* ptr, size_t remain, bool take_ownership) {
    FieldUpdateOP opfield;
    while(remain) {
      switch(read_type(&ptr, &remain)) {
        case Type::INT64: {
          opfield.field = new Field_INT64(&ptr, &remain);
          opfield.ufield = new FieldUpdate_MATH(&ptr, &remain);
          break;
        }
        case Type::DOUBLE: {
          opfield.field = new Field_DOUBLE(&ptr, &remain);
          opfield.ufield = new FieldUpdate_MATH(&ptr, &remain);
          break;
        }
        case Type::BYTES: {
          opfield.field = new Field_BYTES(&ptr, &remain);
          opfield.ufield = new FieldUpdate_LIST(&ptr, &remain);
          break;
        }
        case Type::KEY: {
          opfield.field = new Field_KEY(&ptr, &remain, take_ownership);
          break;
        }
        case Type::LIST_INT64: {
          opfield.field = new Field_LIST_INT64(&ptr, &remain, take_ownership);
          break;
        }
        case Type::LIST_BYTES: {
          opfield.field = new Field_LIST_BYTES(&ptr, &remain, take_ownership);
          break;
        }
        default:
          continue;
      }
      if(!opfield)
        continue;

      auto& typ_fields = (*this)[opfield.field->type()];
      if(typ_fields) {
        count += typ_fields->insert_or_assign(
          uint32_t(opfield.field->fid),
          std::move(opfield)
        ).second;
      } else {
        typ_fields = new std::map<uint32_t, FieldUpdateOP>();
        (*typ_fields)[uint32_t(opfield.field->fid)] = std::move(opfield);
        ++count;
      }
    }
  }

  FieldUpdateOP*
  find_matching_type_and_id(const Field* field) const noexcept {
    auto& typ_fields = (*this)[field->type()];
    if(typ_fields) {
      const auto it = typ_fields->find(field->fid);
      if(it != typ_fields->cend()) {
        return &it->second;
      }
    }
    return nullptr;
  }

  void get_not_in(const FieldUpdateOpPtrs& opfields,
                  FieldUpdateOpPtrs& not_in) const {
    for(auto it_t = cbegin() + 1; it_t != cend(); ++it_t) {
      if(!*it_t)
        continue;
      for(auto it_f = (*it_t)->cbegin(); it_f != (*it_t)->cend(); ++it_f) {
        if(std::find(opfields.cbegin(), opfields.cend(), &it_f->second)
            == opfields.cend()) {
          not_in.push_back(&it_f->second);
        }
      }
    }
  }

};
//



}}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellValueSerialFields.cc"
#endif

#endif // swcdb_db_cells_CellValueSerialFields_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsValueSerialFields_h
#define swcdb_db_cells_SpecsValueSerialFields_h


#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/Cells/SpecsValue.h"
#include "swcdb/db/Cells/SpecsKey.h"


namespace SWC { namespace DB { namespace Specs {



//! The SWC-DB Serial Specifications C++ namespace 'SWC::DB::Specs::Serial'
namespace Serial {



//! The SWC-DB Serial Value Specifications C++ namespace 'SWC::DB::Specs::Serial::Value'
namespace Value {



/* SERIALIZATION:
    [(type, fid, FieldEnc)]
*/

using Type = Cell::Serial::Value::Type;



// Field-Interface with field index
struct Field {
  typedef std::unique_ptr<Field> Ptr;

  uint24_t fid;

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

  virtual bool is_matching(Cell::Serial::Value::Field* vfieldp) = 0;

  virtual void print(std::ostream& out) const = 0;

};
//



// Field INT64
struct Field_INT64 : Field {

  SWC_CAN_INLINE
  static Field::Ptr make(uint24_t fid, Condition::Comp comp, int64_t value) {
    return Field::Ptr(new Field_INT64(fid, comp, value));
  }

  Field_INT64(uint24_t fid, Condition::Comp comp, int64_t value);

  Field_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_INT64() noexcept { }

  Type type() const noexcept override { return Type::INT64; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool SWC_PURE_FUNC is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  int64_t         value;
};
//



// Field DOUBLE
struct Field_DOUBLE : Field {

  SWC_CAN_INLINE
  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const long double& value) {
    return Field::Ptr(new Field_DOUBLE(fid, comp, value));
  }

  Field_DOUBLE(uint24_t fid, Condition::Comp comp, const long double& value);

  Field_DOUBLE(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_DOUBLE() noexcept { }

  Type type() const noexcept override { return Type::DOUBLE; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  long double     value;
};
//



// Field BYTES
struct Field_BYTES : Field {

  SWC_CAN_INLINE
  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const uint8_t* ptr, size_t len) {
    return Field::Ptr(new Field_BYTES(fid, comp, ptr, len, true));
  }

  SWC_CAN_INLINE
  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const std::string& value) {
    return make(
      fid, comp,
      reinterpret_cast<const uint8_t*>(value.c_str()), value.size());
  }

  Field_BYTES(uint24_t fid, Condition::Comp comp,
              const uint8_t* ptr, size_t len, bool take_ownership);

  Field_BYTES(const uint8_t** bufp, size_t* remainp, bool take_ownership);

  virtual ~Field_BYTES() noexcept { }

  Type type() const noexcept override { return Type::BYTES; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  StaticBuffer    value;
};
//



// Field KEY
struct Field_KEY : Field {

  SWC_CAN_INLINE
  static std::unique_ptr<Field_KEY>
  make(uint24_t fid, Types::KeySeq seq) {
    return std::unique_ptr<Field_KEY>(new Field_KEY(fid, seq));
  }

  SWC_CAN_INLINE
  static Field::Ptr
  make(uint24_t fid, Types::KeySeq seq, const Key& key) {
    return Field::Ptr(new Field_KEY(fid, seq, key));
  }

  Field_KEY(uint24_t fid, Types::KeySeq seq);

  Field_KEY(uint24_t fid, Types::KeySeq seq, const Key& key);

  Field_KEY(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_KEY() noexcept { }

  Type type() const noexcept override { return Type::KEY; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Types::KeySeq seq;
  Key           key;

};
//



// Field LIST_INT64
struct Field_LIST_INT64 : Field {

  struct Item {
    Condition::Comp comp;
    int64_t         value;
    SWC_CAN_INLINE
    Item() noexcept : comp(Condition::NONE), value() { }
    constexpr SWC_CAN_INLINE
    Item(Condition::Comp a_comp, int64_t a_value) noexcept
          : comp(a_comp), value(a_value) { }
  };
  typedef Core::Vector<Item> Vec;

  SWC_CAN_INLINE
  static std::unique_ptr<Field_LIST_INT64>
  make(uint24_t fid, Condition::Comp comp) {
    return std::unique_ptr<Field_LIST_INT64>(new Field_LIST_INT64(fid, comp));
  }

  SWC_CAN_INLINE
  static Field::Ptr
  make(uint24_t fid, Condition::Comp comp, const Vec& items) {
    return Field::Ptr(new Field_LIST_INT64(fid, comp, items));
  }

  Field_LIST_INT64(uint24_t fid, Condition::Comp comp);

  Field_LIST_INT64(uint24_t fid, Condition::Comp comp, const Vec& items);

  Field_LIST_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_LIST_INT64() noexcept { }

  Type type() const noexcept override { return Type::LIST_INT64; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp     comp;
  Vec                 items;
  Core::Vector<bool> _found;

};
//



// Field LIST_BYTES
struct Field_LIST_BYTES : Field {

  struct Item {
    Condition::Comp comp;
    std::string     value;
    SWC_CAN_INLINE
    Item() noexcept : comp(Condition::NONE), value() { }
    Item(Item&&) noexcept = default;
    Item(const Item&) = default;
    Item& operator=(Item&&) noexcept = delete;
    Item& operator=(const Item&) = delete;
    SWC_CAN_INLINE
    ~Item() noexcept { }
    SWC_CAN_INLINE
    Item(Condition::Comp a_comp, const std::string& a_value)
        : comp(a_comp), value(a_value) { }
  };
  typedef Core::Vector<Item> Vec;

  SWC_CAN_INLINE
  static std::unique_ptr<Field_LIST_BYTES>
  make(uint24_t fid, Condition::Comp comp) {
    return std::unique_ptr<Field_LIST_BYTES>(new Field_LIST_BYTES(fid, comp));
  }

  SWC_CAN_INLINE
  static Field::Ptr
  make(uint24_t fid, Condition::Comp comp, const Vec& items) {
    return Field::Ptr(new Field_LIST_BYTES(fid, comp, items));
  }

  Field_LIST_BYTES(uint24_t fid, Condition::Comp comp);

  Field_LIST_BYTES(uint24_t fid, Condition::Comp comp, const Vec& items);

  Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_LIST_BYTES() noexcept { }

  Type type() const noexcept override { return Type::LIST_BYTES; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp     comp;
  Vec                 items;
  Core::Vector<bool> _found;

};
//



//
struct Fields {

  SWC_CAN_INLINE
  Fields() noexcept : fields(), _fields_ptr() { }

  Fields(const uint8_t* ptr, size_t len, bool with_state, bool own);

  ~Fields() noexcept { }

  bool SWC_PURE_FUNC has_field_id(uint24_t fid) const noexcept;

  void add(Field::Ptr&& field);

  size_t encoded_length() const noexcept;

  void encode(Specs::Value& value) const;

  void encode(uint8_t** bufp) const;

  bool is_matching(const Cells::Cell& cell);

  bool is_matching_or(const Cells::Cell& cell);

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

  void print(std::ostream& out) const;

  Core::Vector<Field::Ptr> fields;
  Core::Vector<Field*>    _fields_ptr;

};
//



}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValueSerialFields.cc"
#endif

#endif // swcdb_db_cells_SpecsValueSerialFields_h

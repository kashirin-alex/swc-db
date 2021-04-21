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

  Field(uint24_t fid) noexcept : fid(fid) { }

  Field(const uint8_t** bufp, size_t* remainp);

  virtual ~Field() { }

  virtual Type type() const noexcept = 0;

  virtual size_t encoded_length() const noexcept;

  virtual void encode(uint8_t** bufp) const = 0;

  void encode(uint8_t** bufp, Type type) const;

  virtual bool is_matching(Cell::Serial::Value::Field* vfieldp) = 0;

  virtual void print(std::ostream& out) const = 0;

};
//



// Field INT64
struct Field_INT64 : Field {

  static Field::Ptr make(uint24_t fid, Condition::Comp comp, int64_t value) {
    return std::make_unique<Field_INT64>(fid, comp, value);
  }

  Field_INT64(uint24_t fid, Condition::Comp comp, int64_t value)
             : Field(fid), comp(comp), value(value) { }

  Field_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_INT64() { }

  Type type() const noexcept override { return Type::INT64; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  int64_t         value;
};
//



// Field DOUBLE
struct Field_DOUBLE : Field {

  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const long double& value) {
    return std::make_unique<Field_DOUBLE>(fid, comp, value);
  }

  Field_DOUBLE(uint24_t fid, Condition::Comp comp, const long double& value)
             : Field(fid), comp(comp), value(value) { }

  Field_DOUBLE(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_DOUBLE() { }

  Type type() const noexcept override { return Type::DOUBLE; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  long double     value;
};
//



// Field BYTES
struct Field_BYTES : Field {

  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const uint8_t* ptr, size_t len) {
    return std::make_unique<Field_BYTES>(fid, comp, ptr, len, true);
  }

  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const std::string& value) {
    return make(
      fid, comp,
      reinterpret_cast<const uint8_t*>(value.c_str()), value.size());
  }

  Field_BYTES(uint24_t fid, Condition::Comp comp,
              const uint8_t* ptr, size_t len, bool take_ownership=false);

  Field_BYTES(const uint8_t** bufp, size_t* remainp,
              bool take_ownership=false);

  virtual ~Field_BYTES() { }

  Type type() const noexcept override { return Type::BYTES; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  StaticBuffer    value;
};
//



// Field KEY
struct Field_KEY : Field {

  static std::unique_ptr<Field_KEY>
  make(uint24_t fid, Types::KeySeq seq) {
    return std::make_unique<Field_KEY>(fid, seq);
  }

  static Field::Ptr
  make(uint24_t fid, Types::KeySeq seq, const Key& key) {
    return std::make_unique<Field_KEY>(fid, seq, key);
  }

  Field_KEY(uint24_t fid, Types::KeySeq seq)
           : Field(fid), seq(seq) {
  }

  Field_KEY(uint24_t fid, Types::KeySeq seq, const Key& key);

  Field_KEY(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_KEY() { }

  Type type() const noexcept override { return Type::KEY; };

  size_t encoded_length() const noexcept override;

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
    Item() noexcept { }
    Item(Condition::Comp comp, int64_t value) noexcept
          : comp(comp), value(value) { }
  };

  static std::unique_ptr<Field_LIST_INT64>
  make(uint24_t fid, Condition::Comp comp) {
    return std::make_unique<Field_LIST_INT64>(fid, comp);
  }

  static Field::Ptr
  make(uint24_t fid, Condition::Comp comp, const std::vector<Item>& items) {
    return std::make_unique<Field_LIST_INT64>(fid, comp, items);
  }

  Field_LIST_INT64(uint24_t fid, Condition::Comp comp)
                  : Field(fid), comp(comp) {
  }

  Field_LIST_INT64(uint24_t fid, Condition::Comp comp,
                   const std::vector<Item>& items);

  Field_LIST_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_LIST_INT64() { }

  Type type() const noexcept override { return Type::LIST_INT64; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp   comp;
  std::vector<Item> items;

};
//



// Field LIST_BYTES
struct Field_LIST_BYTES : Field {

  struct Item {
    Condition::Comp comp;
    std::string     value;
    Item() noexcept { }
    Item(Condition::Comp comp, const std::string& value)
        : comp(comp), value(value) { }
  };

  static std::unique_ptr<Field_LIST_BYTES>
  make(uint24_t fid, Condition::Comp comp) {
    return std::make_unique<Field_LIST_BYTES>(fid, comp);
  }

  static Field::Ptr
  make(uint24_t fid, Condition::Comp comp, const std::vector<Item>& items) {
    return std::make_unique<Field_LIST_BYTES>(fid, comp, items);
  }

  Field_LIST_BYTES(uint24_t fid, Condition::Comp comp)
                  : Field(fid), comp(comp) {
  }

  Field_LIST_BYTES(uint24_t fid, Condition::Comp comp,
                   const std::vector<Item>& items);

  Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_LIST_BYTES() { }

  Type type() const noexcept override { return Type::LIST_BYTES; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp   comp;
  std::vector<Item> items;

};
//



//
struct Fields {

  Fields() noexcept { }

  Fields(const uint8_t* ptr, size_t len);

  bool has_field_id(uint24_t fid) const noexcept;

  void add(Field::Ptr&& field);

  size_t encoded_length() const noexcept;

  void encode(Specs::Value& value) const;

  void encode(uint8_t** bufp) const;

  bool is_matching(const Cells::Cell& cell);

  std::string to_string() const;

  void print(std::ostream& out) const;

  std::vector<Field::Ptr> fields;

};
//



}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValueSerialFields.cc"
#endif

#endif // swcdb_db_cells_SpecsValueSerialFields_h

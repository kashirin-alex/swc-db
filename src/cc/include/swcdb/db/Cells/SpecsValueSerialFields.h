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
namespace Serial { namespace Value {

/* SERIALIZATION:
    [(type, fid, FieldEnc)]
*/

using Type = Cell::Serial::Value::Type;



// Field-Interface with field index
struct Field {
  typedef std::unique_ptr<Field> Ptr;

  uint24_t fid;

  Field(uint24_t fid) : fid(fid) { }

  Field(const uint8_t** bufp, size_t* remainp);

  virtual ~Field() { }

  virtual Type type() const = 0;

  virtual size_t encoded_length() const;

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

  Type type() const override { return Type::INT64; };

  size_t encoded_length() const override;

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

  Type type() const override { return Type::DOUBLE; };

  size_t encoded_length() const override;

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

  Field_BYTES(uint24_t fid, Condition::Comp comp,
              const uint8_t* ptr, size_t len, bool take_ownership=false);

  Field_BYTES(const uint8_t** bufp, size_t* remainp,
              bool take_ownership=false);

  virtual ~Field_BYTES() { }

  Type type() const override { return Type::BYTES; };

  size_t encoded_length() const override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp comp;
  StaticBuffer    value;
};
//



// Field KEY
struct Field_KEY : Field {

  static Field::Ptr make(uint24_t fid, Types::KeySeq seq, const Key& key) {
    return std::make_unique<Field_KEY>(fid, seq, key);
  }

  Field_KEY(uint24_t fid, Types::KeySeq seq, const Key& key);

  Field_KEY(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_KEY() { }

  Type type() const override { return Type::KEY; };

  size_t encoded_length() const override;

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
    Item() { }
    Item(Condition::Comp comp, int64_t value) : comp(comp), value(value) { }
  };

  static Field::Ptr make(uint24_t fid, Condition::Comp comp,
                         const std::vector<Item>& items) {
    return std::make_unique<Field_LIST_INT64>(fid, comp, items);
  }

  Field_LIST_INT64(uint24_t fid, Condition::Comp comp,
                   const std::vector<Item>& items);

  Field_LIST_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_LIST_INT64() { }

  Type type() const override { return Type::LIST_INT64; };

  size_t encoded_length() const override;

  void encode(uint8_t** bufp) const override;

  bool is_matching(Cell::Serial::Value::Field* vfieldp) override;

  void print(std::ostream& out) const override;

  Condition::Comp   comp;
  std::vector<Item> items;

};
//



//
struct Fields {

  Fields() { }

  Fields(const uint8_t* ptr, size_t len);

  bool has_field_id(uint24_t fid) const;

  void add(Field::Ptr&& field);

  size_t encoded_length() const;

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

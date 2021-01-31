/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellValueSerialFields_h
#define swcdb_db_cells_CellValueSerialFields_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace DB { namespace Cell {
namespace Serial { namespace Value {

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

const char* to_string(Type typ) noexcept;

Type read_type(const uint8_t** bufp, size_t* remainp);
//



// Field-Base
struct Field {

  uint24_t fid;

  Field() noexcept { }

  Field(uint24_t fid) noexcept : fid(fid) { }

  Field(const uint8_t** bufp, size_t* remainp);

  virtual ~Field() { }

  virtual Type type() const noexcept = 0;

  virtual size_t encoded_length() const noexcept;

  virtual void encode(uint8_t** bufp) const = 0;

  virtual void encode(uint8_t** bufp, Type type) const;

  virtual void print(std::ostream& out) const = 0;

};
//



// Field INT64
struct Field_INT64 : Field {

  int64_t value;

  Field_INT64() noexcept { }

  Field_INT64(uint24_t fid, int64_t value) noexcept
              : Field(fid), value(value) { }

  Field_INT64(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_INT64() { }

  Type type() const noexcept override { return Type::INT64; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void print(std::ostream& out) const override;

};
//



// Field DOUBLE
struct Field_DOUBLE : Field {

  long double value;

  Field_DOUBLE() noexcept { }

  Field_DOUBLE(uint24_t fid, const long double& value) noexcept
              : Field(fid), value(value) { }

  Field_DOUBLE(const uint8_t** bufp, size_t* remainp);

  virtual ~Field_DOUBLE() { }

  Type type() const noexcept override { return Type::DOUBLE; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void print(std::ostream& out) const override;

};
//



// Field BYTES
struct Field_BYTES : Field, StaticBuffer {

  Field_BYTES() noexcept { }

  Field_BYTES(uint24_t fid, const uint8_t* data, uint32_t len,
              bool take_ownership=false);

  Field_BYTES(const uint8_t** bufp, size_t* remainp,
              bool take_ownership=false);

  virtual ~Field_BYTES() { }

  Type type() const noexcept override { return Type::BYTES; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void convert_to(std::string& item) const;

  void print(std::ostream& out) const override;

};
//



// Field KEY
struct Field_KEY : Field {

  Field_KEY() noexcept { }

  Field_KEY(uint24_t fid, const Key& key, bool take_ownership=false);

  Field_KEY(const uint8_t** bufp, size_t* remainp, bool take_ownership=false);

  virtual ~Field_KEY() { }

  Type type() const noexcept override { return Type::KEY; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void print(std::ostream& out) const override;

  Key key;

};
//



// Field LIST_INT64
struct Field_LIST_INT64 : Field, StaticBuffer {

  Field_LIST_INT64() noexcept { }

  Field_LIST_INT64(uint24_t fid, const std::vector<int64_t>& items);

  Field_LIST_INT64(const uint8_t** bufp, size_t* remainp,
                   bool take_ownership=false);

  virtual ~Field_LIST_INT64() { }

  Type type() const noexcept override { return Type::LIST_INT64; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void convert_to(std::vector<int64_t>& items) const;

  void print(std::ostream& out) const override;

};
//



// Field LIST_BYTES
struct Field_LIST_BYTES : Field, StaticBuffer {

  Field_LIST_BYTES() noexcept { }

  Field_LIST_BYTES(uint24_t fid, const std::vector<std::string>& items);

  Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp,
                   bool take_ownership=false);

  virtual ~Field_LIST_BYTES() { }

  Type type() const noexcept override { return Type::LIST_BYTES; };

  size_t encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  void convert_to(std::vector<std::string>& items) const;

  void print(std::ostream& out) const override;

};
//



/*
FieldsWriter wfields;
wfields.add(123);

cell.set_value(wfields.base, wfields.fill(), true);
or
cell.set_value(encoder, wfields.base, wfields.fill());
*/
struct FieldsWriter final : DynamicBuffer {

  FieldsWriter() noexcept : index_count(0) { }

  ~FieldsWriter() { }

  void add(Field* field);


  void add(const int64_t& value);

  void add(uint24_t fid, const int64_t& value);


  void add(const long double& value);

  void add(uint24_t fid, const long double& value);


  void add(const uint8_t* data, uint32_t len);

  void add(uint24_t fid, const uint8_t* data, uint32_t len);

  void add(const std::string& data) {
    add(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
  }

  void add(uint24_t fid, const std::string& data) {
    add(fid, reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
  }


  void add(const Key& key);

  void add(uint24_t fid, const Key& key);


  void add(const std::vector<int64_t>& items);

  void add(uint24_t fid, const std::vector<int64_t>& items);


  void add(const std::vector<std::string>& items);

  void add(uint24_t fid, const std::vector<std::string>& items);


  std::string to_string() const;

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



}}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellValueSerialFields.cc"
#endif

#endif // swcdb_db_cells_CellValueSerialFields_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellValueSerialField_h
#define swcdb_db_cells_CellValueSerialField_h

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
    set_from(items);
  }

  Field_LIST_INT64(const uint8_t** bufp, size_t* remainp,
                   bool take_ownership=false);

  virtual ~Field_LIST_INT64() noexcept { }

  Type type() const noexcept override { return Type::LIST_INT64; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  template<typename T>
  void set_from(const T& items) {
    uint32_t len = 0;
    for(const auto& v : items)
      len += Serialization::encoded_length_vi64(v);
    reallocate(len);
    uint8_t* ptr = base;
    for(const auto& v : items)
      Serialization::encode_vi64(&ptr, v);
  }

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
    set_from(items);
  }

  Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp,
                   bool take_ownership=false);

  virtual ~Field_LIST_BYTES() noexcept { }

  Type type() const noexcept override { return Type::LIST_BYTES; };

  size_t SWC_PURE_FUNC encoded_length() const noexcept override;

  void encode(uint8_t** bufp) const override;

  template<typename T>
  void set_from(const T& items) {
    uint32_t len = 0;
    for(auto& v : items)
      len += Serialization::encoded_length_bytes(v.size());
    reallocate(len);
    uint8_t* ptr = base;
    for(auto& v : items)
      Serialization::encode_bytes(&ptr, v.data(), v.size());
  }

  template<typename T>
  void convert_to(T& items) const {
    if(size) {
      const uint8_t* ptr = base;
      for(size_t remain = size; remain;) {
        size_t len;
        auto cptr = reinterpret_cast<
          const typename T::value_type::value_type*>(
            Serialization::decode_bytes(&ptr, &remain, &len));
        items.emplace_back(cptr, len);
      }
    }
  }

  void print(std::ostream& out) const override;

};
//



}}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellValueSerialField.cc"
#endif

#endif // swcdb_db_cells_CellValueSerialField_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cell {
namespace Serial { namespace Value {


const char* to_string(Type typ) noexcept {
  switch(typ) {
    case Type::INT64:
      return "INT64";
    case Type::DOUBLE:
      return "DOUBLE";
    case Type::BYTES:
      return "BYTES";
    case Type::KEY:
      return "KEY";
    case Type::LIST_INT64:
      return "LIST_INT64";
    case Type::LIST_BYTES:
      return "LIST_BYTES";
    /*
    case Type::MAP:
      return "MAP";
    case Type::LIST:
      return "LIST";
    */
    default:
      return "UNKNOWN";
  }
}
//





// Field INT64
Field_INT64::Field_INT64(const uint8_t** bufp, size_t* remainp)
                        : Field(bufp, remainp),
                          value(Serialization::decode_vi64(bufp, remainp)) {
}

size_t Field_INT64::encoded_length() const noexcept {
  return Field::encoded_length() +
         Serialization::encoded_length_vi64(value);
}

void Field_INT64::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::INT64);
  Serialization::encode_vi64(bufp, value);
}

void Field_INT64::print(std::ostream& out) const {
  out << fid << ':' << 'I' << ':' << value;
}
//



// Field DOUBLE
Field_DOUBLE::Field_DOUBLE(const uint8_t** bufp, size_t* remainp)
                        : Field(bufp, remainp),
                          value(Serialization::decode_double(bufp, remainp)) {
}

size_t Field_DOUBLE::encoded_length() const noexcept {
  return Field::encoded_length() +
         Serialization::encoded_length_double();
}

void Field_DOUBLE::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::DOUBLE);
  Serialization::encode_double(bufp, value);
}

void Field_DOUBLE::print(std::ostream& out) const {
  out << fid << ':' << 'D' << ':' << value;
}
//



// Field BYTES
Field_BYTES::Field_BYTES(uint24_t a_fid,
                         const uint8_t* data, uint32_t len,
                         bool take_ownership)
                        : Field(a_fid) {
  take_ownership
    ? assign(data, len)
    : set(const_cast<uint8_t*>(data), len, false);
}

Field_BYTES::Field_BYTES(const uint8_t** bufp, size_t* remainp,
                         bool take_ownership)
                        : Field(bufp, remainp) {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  take_ownership
    ? assign(ptr, len)
    : set(const_cast<uint8_t*>(ptr), len, false);
}

size_t Field_BYTES::encoded_length() const noexcept {
  return Field::encoded_length() +
         Serialization::encoded_length_bytes(size);
}

void Field_BYTES::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::BYTES);
  Serialization::encode_bytes(bufp, base, size);
}

void Field_BYTES::print(std::ostream& out) const {
  out << fid << ':' << 'B' << ':' << '"';
  char hex[5];
  hex[4] = '\0';
  const uint8_t* end = base + size;
  for(const uint8_t* ptr = base; ptr < end; ++ptr) {
    if(*ptr == '"')
      out << '\\';
    if(31 < *ptr && *ptr < 127) {
      out << *ptr;
    } else {
      sprintf(hex, "0x%X", *ptr);
      out << hex;
    }
  }
  out << '"';
}
//



// Field KEY
Field_KEY::Field_KEY(uint24_t a_fid, const Key& a_key, bool take_ownership)
                    : Field(a_fid), key(a_key, take_ownership) {
}

Field_KEY::Field_KEY(const uint8_t** bufp, size_t* remainp,
                     bool take_ownership)
                    : Field(bufp, remainp) {
  key.decode(bufp, remainp, take_ownership);
}

void Field_KEY::decode(const uint8_t** bufp, size_t* remainp,
                       bool take_ownership) {
  Field::decode(bufp, remainp);
  key.decode(bufp, remainp, take_ownership);
}

size_t Field_KEY::encoded_length() const noexcept {
  return Field::encoded_length() +
         key.encoded_length();
}

void Field_KEY::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::KEY);
  key.encode(bufp);
}

void Field_KEY::print(std::ostream& out) const {
  out << fid << ':' << 'K' << ':';
  key.display(out);
}
//



// Field LIST_INT64
Field_LIST_INT64::Field_LIST_INT64(const uint8_t** bufp, size_t* remainp,
                                   bool take_ownership)
                                  : Field(bufp, remainp) {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  take_ownership
    ? assign(ptr, len)
    : set(const_cast<uint8_t*>(ptr), len, false);
}

size_t Field_LIST_INT64::encoded_length() const noexcept {
  return Field::encoded_length() +
         Serialization::encoded_length_bytes(size);
}

void Field_LIST_INT64::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::LIST_INT64);
  Serialization::encode_bytes(bufp, base, size);
}

void Field_LIST_INT64::print(std::ostream& out) const {
  out << fid << ":LI:[";
  if(size) {
    const uint8_t* ptr = base;
    for(size_t remain = size;;) {
      out << Serialization::decode_vi64(&ptr, &remain);
      if(!remain)
        break;
      out << ',';
    }
  }
  out << ']';
}
//



// Field LIST_BYTES
Field_LIST_BYTES::Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp,
                                   bool take_ownership)
                                  : Field(bufp, remainp) {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  take_ownership
    ? assign(ptr, len)
    : set(const_cast<uint8_t*>(ptr), len, false);
}

size_t Field_LIST_BYTES::encoded_length() const noexcept {
  return Field::encoded_length() +
         Serialization::encoded_length_bytes(size);
}

void Field_LIST_BYTES::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::LIST_BYTES);
  Serialization::encode_bytes(bufp, base, size);
}

void Field_LIST_BYTES::print(std::ostream& out) const {
  out << fid << ":LB:[";
  if(size) {
    const uint8_t* ptr = base;
    for(size_t remain = size;;) {
      out << '\'';
      char hex[5];
      hex[4] = '\0';
      size_t len;
      const uint8_t* cptr = Serialization::decode_bytes(&ptr, &remain, &len);
      const uint8_t* end = cptr + len;
      for(; cptr < end; ++cptr) {
        if(*cptr == '"')
          out << '\\';
        if(31 < *cptr && *cptr < 127) {
          out << *cptr;
        } else {
          sprintf(hex, "0x%X", *cptr);
          out << hex;
        }
      }
      out << '\'';
      if(!remain)
        break;
      out << ',';
    }
  }
  out << ']';
}
//



//
void FieldsWriter::add(uint24_t fid, const int64_t& value) {
  Field_INT64 field(fid, value);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const long double& value) {
  Field_DOUBLE field(fid, value);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const uint8_t* data, uint32_t len) {
  Field_BYTES field(fid, data, len);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const Key& key) {
  Field_KEY field(fid, key);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const Core::Vector<int64_t>& items) {
  Field_LIST_INT64 field(fid, items);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const std::vector<int64_t>& items) {
  Field_LIST_INT64 field(fid, items);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const Core::Vector<std::string>& items) {
  Field_LIST_BYTES field(fid, items);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const std::vector<std::string>& items) {
  Field_LIST_BYTES field(fid, items);
  add(&field);
}

void FieldsWriter::print(std::ostream& out) const {
  if(!fill()) {
    out << "SerialFields(size=0)";
  } else {
    Fields::print(base, fill(), out);
  }
}
//



//
void Fields::print(const uint8_t* ptr, size_t remain, std::ostream& out) {
  out << "SerialFields(";
  display(ptr, remain, out);
  out << ")";
}

void Fields::display(const uint8_t* ptr, size_t remain, std::ostream& out) {
  out << '[';
  if(remain) for(;;) {
    switch(read_type(&ptr, &remain)) {
      case Type::INT64:
        Field_INT64(&ptr, &remain).print(out);
        break;
      case Type::DOUBLE:
        Field_DOUBLE(&ptr, &remain).print(out);
        break;
      case Type::BYTES:
        Field_BYTES(&ptr, &remain).print(out);
        break;
      case Type::KEY:
        Field_KEY(&ptr, &remain).print(out);
        break;
      case Type::LIST_INT64:
        Field_LIST_INT64(&ptr, &remain).print(out);
        break;
      case Type::LIST_BYTES:
        Field_LIST_BYTES(&ptr, &remain).print(out);
        break;
      default:
        out << "Type Unknown - corrupted remain=" << remain;
        remain = 0;
        break;
    }
    if(!remain)
      break;
    out << ',';
  }
  out << ']';
}
///



}}}}}

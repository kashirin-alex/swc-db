/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cell {
namespace Serial { namespace Value {



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
  Field_BYTES field(fid, data, len, false);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const Key& key) {
  Field_KEY field(fid, key, false);
  add(&field);
}

void FieldsWriter::add(uint24_t fid, const Core::Vector<int64_t>& items) {
  Field_LIST_INT64 field(fid);
  field.write(this, items);
}

void FieldsWriter::add(uint24_t fid, const std::vector<int64_t>& items) {
  Field_LIST_INT64 field(fid);
  field.write(this, items);
}

void FieldsWriter::add(uint24_t fid, const Core::Vector<std::string>& items) {
  Field_LIST_BYTES field(fid);
  field.write(this, items);
}

void FieldsWriter::add(uint24_t fid, const std::vector<std::string>& items) {
  Field_LIST_BYTES field(fid);
  field.write(this, items);
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
        Field_BYTES(&ptr, &remain, false).print(out);
        break;
      case Type::KEY:
        Field_KEY(&ptr, &remain, false).print(out);
        break;
      case Type::LIST_INT64:
        Field_LIST_INT64(&ptr, &remain, false).print(out);
        break;
      case Type::LIST_BYTES:
        Field_LIST_BYTES(&ptr, &remain, false).print(out);
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

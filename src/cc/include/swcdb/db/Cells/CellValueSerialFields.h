/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellValueSerialFields_h
#define swcdb_db_cells_CellValueSerialFields_h

#include "swcdb/db/Cells/CellValueSerialFieldUpdate.h"


namespace SWC { namespace DB { namespace Cell {



//! The SWC-DB Serial Cell C++ namespace 'SWC::DB::Cell::Serial'
namespace Serial {



//! The SWC-DB Serial Value Cell C++ namespace 'SWC::DB::Cell::Serial::Value'
namespace Value {


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

  SWC_CAN_INLINE
  void add(Field* field, FieldUpdate* ufield) {
    ensure(field->encoded_length() + ufield->encoded_length());
    field->encode(&ptr);
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



/// Fields Reader with FieldsWriter & FieldUpdate serialization
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
  SWC_CAN_INLINE
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
    while(remain) {
      FieldUpdateOP opfield;
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
          opfield.field = new Field_BYTES(&ptr, &remain, take_ownership);
          opfield.ufield = new FieldUpdate_LIST(&ptr, &remain);
          break;
        }
        case Type::KEY: {
          opfield.field = new Field_KEY(&ptr, &remain, take_ownership);
          break;
        }
        case Type::LIST_INT64: {
          opfield.field = new Field_LIST_INT64(&ptr, &remain, take_ownership);
          opfield.ufield = new FieldUpdate_LIST_INT64(&ptr, &remain);
          break;
        }
        case Type::LIST_BYTES: {
          opfield.field = new Field_LIST_BYTES(&ptr, &remain, take_ownership);
          opfield.ufield = new FieldUpdate_LIST_BYTES(&ptr, &remain);
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

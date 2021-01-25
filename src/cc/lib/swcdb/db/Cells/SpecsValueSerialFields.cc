/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsValueSerialFields.h"


namespace SWC { namespace DB { namespace Specs {
namespace Serial { namespace Value {



// Field Base
Field::Field(const uint8_t** bufp, size_t* remainp)
            : fid(Serialization::decode_vi24(bufp, remainp)) {
}

size_t Field::encoded_length() const {
  return 1 + Serialization::encoded_length_vi24(fid);
}

void Field::encode(uint8_t** bufp, Type type) const {
  Serialization::encode_i8(bufp, type);
  Serialization::encode_vi24(bufp, fid);
}
//



// Field INT64
Field_INT64::Field_INT64(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          value(Serialization::decode_vi64(bufp, remainp)) {
}

size_t Field_INT64::encoded_length() const {
  return Field::encoded_length()
       + 1 + Serialization::encoded_length_vi64(value);
}

void Field_INT64::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::INT64);
  Serialization::encode_i8(bufp, comp);
  Serialization::encode_vi64(bufp, value);
}

bool Field_INT64::is_matching(Cell::Serial::Value::Field* vfieldp) {
  return Condition::is_matching(
    comp, value,
    static_cast<Cell::Serial::Value::Field_INT64*>(vfieldp)->value);
}

void Field_INT64::print(std::ostream& out) const {
  out << fid << ':' << 'I' << ':'
      << Condition::to_string(comp, true) << value;
}
//



// Field DOUBLE
Field_DOUBLE::Field_DOUBLE(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          value(Serialization::decode_double(bufp, remainp)) {
}

size_t Field_DOUBLE::encoded_length() const {
  return Field::encoded_length()
       + 1 + Serialization::encoded_length_double();
}

void Field_DOUBLE::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::DOUBLE);
  Serialization::encode_i8(bufp, comp);
  Serialization::encode_double(bufp, value);
}

bool Field_DOUBLE::is_matching(Cell::Serial::Value::Field* vfieldp) {
  return Condition::is_matching(
    comp, value,
    static_cast<Cell::Serial::Value::Field_DOUBLE*>(vfieldp)->value);
}

void Field_DOUBLE::print(std::ostream& out) const {
  out << fid << ':' << 'D' << ':'
      << Condition::to_string(comp, true) << value;
}
//



// Field BYTES
Field_BYTES::Field_BYTES(uint24_t fid, Condition::Comp comp,
                         const uint8_t* ptr, size_t len,
                         bool take_ownership)
                        : Field(fid), comp(comp) {
  take_ownership
    ? value.assign(ptr, len)
    : value.set(const_cast<uint8_t*>(ptr), len, false);
}

Field_BYTES::Field_BYTES(const uint8_t** bufp, size_t* remainp,
                         bool take_ownership)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))) {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  take_ownership
    ? value.assign(ptr, len)
    : value.set(const_cast<uint8_t*>(ptr), len, false);
}

size_t Field_BYTES::encoded_length() const {
  return Field::encoded_length()
       + 1 + Serialization::encoded_length_bytes(value.size);
}

void Field_BYTES::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::BYTES);
  Serialization::encode_i8(bufp, comp);
  Serialization::encode_bytes(bufp, value.base, value.size);
}

bool Field_BYTES::is_matching(Cell::Serial::Value::Field* vfieldp) {
  auto vfield = static_cast<Cell::Serial::Value::Field_BYTES*>(vfieldp);
  // compiled RE
  return Condition::is_matching_extended(
    comp, value.base, value.size, vfield->base, vfield->size);
}

void Field_BYTES::print(std::ostream& out) const {
  out << fid << ':' << 'B' << ':'
      << Condition::to_string(comp, true) << '"';
  char hex[5];
  hex[4] = '\0';
  const uint8_t* end = value.base + value.size;
  for(const uint8_t* ptr = value.base; ptr < end; ++ptr) {
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
Field_KEY::Field_KEY(uint24_t fid, Types::KeySeq seq, const Key& key)
                    : Field(fid), seq(seq), key(key) {
}

Field_KEY::Field_KEY(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          seq(Types::KeySeq(Serialization::decode_i8(bufp, remainp))) {
  key.decode(bufp, remainp);
}

size_t Field_KEY::encoded_length() const {
  return Field::encoded_length() + 1 + key.encoded_length();
}

void Field_KEY::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::KEY);
  Serialization::encode_i8(bufp, uint8_t(seq));
  key.encode(bufp);
}

bool Field_KEY::is_matching(Cell::Serial::Value::Field* vfieldp) {
  return key.is_matching(
    seq, static_cast<Cell::Serial::Value::Field_KEY*>(vfieldp)->key);
}

void Field_KEY::print(std::ostream& out) const {
  out << fid << ':' << 'K' << ':';
  key.display(out);
}
//



// Field LIST_INT64
Field_LIST_INT64::Field_LIST_INT64(uint24_t fid, Condition::Comp comp,
                                   const std::vector<Item>& items)
                                  : Field(fid),
                                    comp(comp), items(items) {
}

Field_LIST_INT64::Field_LIST_INT64(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))) {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  while(len) {
    auto vcomp = Condition::Comp(Serialization::decode_i8(&ptr, &len));
    items.emplace_back(vcomp, Serialization::decode_vi64(&ptr, &len));
  }
}

size_t Field_LIST_INT64::encoded_length() const {
  size_t len = items.size();
  for(const Item& item : items)
    len += Serialization::encoded_length_vi64(item.value);
  return Field::encoded_length()
        + 1 + Serialization::encoded_length_bytes(len);
}

void Field_LIST_INT64::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::LIST_INT64);
  Serialization::encode_i8(bufp, comp);

  size_t len = items.size();
  for(const Item& item : items)
    len += Serialization::encoded_length_vi64(item.value);

  StaticBuffer value(len);
  uint8_t* ptr = value.base;
  for(const Item& item : items) {
    Serialization::encode_i8(&ptr, item.comp);
    Serialization::encode_vi64(&ptr, item.value);
  }
  Serialization::encode_bytes(bufp, value.base, value.size);
}

bool Field_LIST_INT64::is_matching(Cell::Serial::Value::Field* vfieldp) {
  auto vfield = static_cast<Cell::Serial::Value::Field_LIST_INT64*>(vfieldp);

  const uint8_t* ptr = vfield->base;
  size_t remain = vfield->size;
  switch(comp) {

    case Condition::NE: {
      auto it = items.begin();
      for(; remain && it < items.end(); ++it) {
        if(!Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain))))
          return true;
      }
      return remain || it < items.end();
    }

    case Condition::GT:
    case Condition::LT:
    case Condition::GE:
    case Condition::LE:
    case Condition::EQ: {
      auto it = items.begin();
      for(; remain && it < items.end(); ++it) {
        if(!Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain))))
          break;
      }
      return remain
        ? (comp == Condition::GT || comp == Condition::GE)
        : (it == items.end()
            ? (comp == Condition::EQ ||
               comp == Condition::LE || comp == Condition::GE)
            : (comp == Condition::LT || comp == Condition::LE));
    }

    case Condition::SBS: {
      std::vector<std::vector<Item>::const_iterator> found(
        items.size(), items.cend());
      while(remain) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(auto it = items.begin(); ; ) {
          if(Condition::is_matching(it->comp, it->value, v)) {
            auto f = found.begin();
            for(; *f != items.cend(); ++f) {
              if(*f == it)
                goto _continue_sbs;
            }
            *f = it;
            if(++f == found.end())
              remain = 0;
            break;
          }
          _continue_sbs:
            if(++it == items.end())
              break;
        }
      }
      for(auto& f : found) {
        if(f == items.cend())
          return false;
      }
      return true;
    }

    case Condition::SPS: {
      std::vector<std::vector<Item>::const_iterator> found(
        items.size(), items.cend());
      while(remain) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(auto it = items.begin(); ;) {
          if(Condition::is_matching(it->comp, it->value, v)) {
            auto f = found.begin();
            for(; *f != items.cend(); ++f) {
              if(*f == it)
                goto _continue_sps;
            }
            *f = it;
            break;
          }
          _continue_sps:
            if(++it == items.end())
              return false;
        }
      }
      return true;
    }

    case Condition::POSBS: {
      auto it = items.begin();
      for(; remain && it < items.end(); ) {
        if(Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain))))
          ++it;
      }
      return it == items.end();
    }

    case Condition::FOSBS: {
      auto it = items.begin();
      for(bool start = false; remain && it < items.end(); ) {
        if(Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain)))) {
          start = true;
          ++it;
        } else if(start) {
          return false;
        }
      }
      return it == items.end();
    }

    case Condition::POSPS: {
      for(auto ord = items.begin(); remain && ord < items.end(); ) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(auto it = ord; ;) {
          if(Condition::is_matching(it->comp, it->value, v)) {
            ++ord;
            break;
          }
          if(++it == items.end())
            return false;
        }
      }
      return !remain;
    }

    case Condition::FOSPS: {
      bool start = false;
      for(auto ord = items.begin(); remain && ord < items.end(); ) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(auto it = ord; ;) {
          if(Condition::is_matching(it->comp, it->value, v)) {
            start = true;
            ord = ++it;
            break;
          } else if(start) {
            return false;
          }
          if(++it == items.end())
            return false;
        }
      }
      return !remain;
    }

    default:
      break;
  }
  return false;
}

void Field_LIST_INT64::print(std::ostream& out) const {
  out << fid << ":LI:" << Condition::to_string(comp, true) << '[';
  if(!items.empty()) for(auto it = items.cbegin(); ;) {
    out << Condition::to_string(it->comp, true) << it->value;
    if(++it == items.cend())
      break;
    out << ',';
  }
  out << ']';
}
//



// Field LIST_BYTES
Field_LIST_BYTES::Field_LIST_BYTES(uint24_t fid, Condition::Comp comp,
                                   const std::vector<Item>& items)
                                  : Field(fid),
                                    comp(comp), items(items) {
}

Field_LIST_BYTES::Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))) {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  while(len) {
    auto& item = items.emplace_back();
    item.comp = Condition::Comp(Serialization::decode_i8(&ptr, &len));
    size_t vlen;
    const uint8_t* vptr = Serialization::decode_bytes(&ptr, &len, &vlen);
    item.value.append(reinterpret_cast<const char*>(vptr), vlen);
  }
}

size_t Field_LIST_BYTES::encoded_length() const {
  size_t len = items.size();
  for(const Item& item : items)
    len += Serialization::encoded_length_bytes(item.value.size());
  return Field::encoded_length()
        + 1 + Serialization::encoded_length_bytes(len);
}

void Field_LIST_BYTES::encode(uint8_t** bufp) const {
  Field::encode(bufp, Type::LIST_BYTES);
  Serialization::encode_i8(bufp, comp);

  size_t len = items.size();
  for(const Item& item : items)
    len += Serialization::encoded_length_bytes(item.value.size());

  StaticBuffer value(len);
  uint8_t* ptr = value.base;
  for(const Item& item : items) {
    Serialization::encode_i8(&ptr, item.comp);
    Serialization::encode_bytes(&ptr, item.value.data(), item.value.size());
  }
  Serialization::encode_bytes(bufp, value.base, value.size);
}

bool Field_LIST_BYTES::is_matching(Cell::Serial::Value::Field* vfieldp) {
  auto vfield = static_cast<Cell::Serial::Value::Field_LIST_BYTES*>(vfieldp);

  const uint8_t* ptr = vfield->base;
  size_t remain = vfield->size;
  switch(comp) {

    case Condition::NE: {
      auto it = items.begin();
      for(; remain && it < items.end(); ++it) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        if(!Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen))
          return true;
      }
      return remain || it < items.end();
    }

    case Condition::GT:
    case Condition::LT:
    case Condition::GE:
    case Condition::LE:
    case Condition::EQ: {
      auto it = items.begin();
      for(; remain && it < items.end(); ++it) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        if(!Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen))
          break;
      }
      return remain
        ? (comp == Condition::GT || comp == Condition::GE)
        : (it == items.end()
            ? (comp == Condition::EQ ||
               comp == Condition::LE || comp == Condition::GE)
            : (comp == Condition::LT || comp == Condition::LE));
    }

    case Condition::SBS: {
      std::vector<std::vector<Item>::const_iterator> found(
        items.size(), items.cend());
      while(remain) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        for(auto it = items.begin(); ; ) {
          if(Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen)) {
            auto f = found.begin();
            for(; *f != items.cend(); ++f) {
              if(*f == it)
                goto _continue_sbs;
            }
            *f = it;
            if(++f == found.end())
              remain = 0;
            break;
          }
          _continue_sbs:
            if(++it == items.end())
              break;
        }
      }
      for(auto& f : found) {
        if(f == items.cend())
          return false;
      }
      return true;
    }

    case Condition::SPS: {
      std::vector<std::vector<Item>::const_iterator> found(
        items.size(), items.cend());
      while(remain) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        for(auto it = items.begin(); ;) {
          if(Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen)) {
            auto f = found.begin();
            for(; *f != items.cend(); ++f) {
              if(*f == it)
                goto _continue_sps;
            }
            *f = it;
            break;
          }
          _continue_sps:
            if(++it == items.end())
              return false;
        }
      }
      return true;
    }

    case Condition::POSBS: {
      auto it = items.begin();
      for(; remain && it < items.end(); ) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        if(Condition::is_matching_extended(
            it->comp,
            reinterpret_cast<const uint8_t*>(it->value.c_str()),
            it->value.size(),
            vptr, vlen))
          ++it;
      }
      return it == items.end();
    }

    case Condition::FOSBS: {
      auto it = items.begin();
      for(bool start = false; remain && it < items.end(); ) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        if(Condition::is_matching_extended(
            it->comp,
            reinterpret_cast<const uint8_t*>(it->value.c_str()),
            it->value.size(),
            vptr, vlen)) {
          start = true;
          ++it;
        } else if(start) {
          return false;
        }
      }
      return it == items.end();
    }

    case Condition::POSPS: {
      for(auto ord = items.begin(); remain && ord < items.end(); ) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        for(auto it = ord; ;) {
          if(Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen)) {
            ++ord;
            break;
          }
          if(++it == items.end())
            return false;
        }
      }
      return !remain;
    }

    case Condition::FOSPS: {
      bool start = false;
      for(auto ord = items.begin(); remain && ord < items.end(); ) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        for(auto it = ord; ;) {
          if(Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen)) {
            start = true;
            ord = ++it;
            break;
          } else if(start) {
            return false;
          }
          if(++it == items.end())
            return false;
        }
      }
      return !remain;
    }

    default:
      break;
  }
  return false;
}

void Field_LIST_BYTES::print(std::ostream& out) const {
  out << fid << ":LB:" << Condition::to_string(comp, true) << '[';
  if(!items.empty()) for(auto it = items.cbegin(); ;) {
    out << Condition::to_string(it->comp, true) << '\'';
    char hex[5];
    hex[4] = '\0';
    const uint8_t* cptr = reinterpret_cast<const uint8_t*>(it->value.c_str());
    const uint8_t* end = cptr + it->value.size();
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
    if(++it == items.cend())
      break;
    out << ',';
  }
  out << ']';
}
//



//
Fields::Fields(const uint8_t* ptr, size_t len) {
  while(len) {
    switch(Cell::Serial::Value::read_type(&ptr, &len)) {
      case Type::INT64: {
        fields.emplace_back(new Field_INT64(&ptr, &len));
        break;
      }
      case Type::DOUBLE: {
        fields.emplace_back(new Field_DOUBLE(&ptr, &len));
        break;
      }
      case Type::BYTES: {
        fields.emplace_back(new Field_BYTES(&ptr, &len));
        break;
      }
      case Type::KEY: {
        fields.emplace_back(new Field_KEY(&ptr, &len));
        break;
      }
      case Type::LIST_INT64: {
        fields.emplace_back(new Field_LIST_INT64(&ptr, &len));
        break;
      }
      case Type::LIST_BYTES: {
        fields.emplace_back(new Field_LIST_BYTES(&ptr, &len));
        break;
      }
      default:
        break;
    }
  }
}

bool Fields::has_field_id(uint24_t fid) const {
  for(auto& field : fields) {
    if(field->fid == fid)
      return true;
  }
  return false;
}

void Fields::add(Field::Ptr&& field) {
  fields.push_back(std::move(field));
}

size_t Fields::encoded_length() const {
  size_t sz = 0;
  for(auto& field : fields)
    sz += field->encoded_length();
  return sz;
}

void Fields::encode(Specs::Value& value) const {
  size_t len = encoded_length();
  if(len) {
    StaticBuffer buffer(len);
    uint8_t* ptr = buffer.base;
    encode(&ptr);
    value.set(buffer.base, buffer.size, value.comp, false);
    buffer.own = false;
    value.own = true;
  } else {
    value.free();
  }
}

void Fields::encode(uint8_t** bufp) const {
  for(auto& field : fields)
    field->encode(bufp);
}

template<Type TypeT>
SWC_CAN_INLINE
bool
is_matching(std::vector<Field*>& fields_ptr,
            Cell::Serial::Value::Field* vfieldp,
            bool* more) {
  for(auto it = fields_ptr.begin(); it < fields_ptr.end(); ++it) {
    if(*it) {
      if((*it)->type() == TypeT && vfieldp->fid == (*it)->fid) {
        if(!(*it)->is_matching(vfieldp))
          return false;
        *it = nullptr;
      } else {
        *more = true;
      }
    }
  }
  return true;
}

bool Fields::is_matching(const Cells::Cell& cell) {
  std::vector<Field*> fields_ptr(fields.size());
  for(size_t i=0; i< fields.size(); ++i)
    fields_ptr[i] = fields[i].get();

  StaticBuffer v;
  cell.get_value(v);

  const uint8_t* ptr = v.base;
  size_t remain = v.size;
  while(remain) {

    bool more = false;
    switch(Cell::Serial::Value::read_type(&ptr, &remain)) {

      case Type::INT64: {
        Cell::Serial::Value::Field_INT64 vfield(&ptr, &remain);
        if(!Value::is_matching<Type::INT64>(fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::DOUBLE: {
        Cell::Serial::Value::Field_DOUBLE vfield(&ptr, &remain);
        if(!Value::is_matching<Type::DOUBLE>(fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::BYTES: {
        Cell::Serial::Value::Field_BYTES vfield(&ptr, &remain);
        if(!Value::is_matching<Type::BYTES>(fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::KEY: {
        Cell::Serial::Value::Field_KEY vfield(&ptr, &remain);
        if(!Value::is_matching<Type::KEY>(fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::LIST_INT64: {
        Cell::Serial::Value::Field_LIST_INT64 vfield(&ptr, &remain);
        if(!Value::is_matching<Type::LIST_INT64>(fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::LIST_BYTES: {
        Cell::Serial::Value::Field_LIST_BYTES vfield(&ptr, &remain);
        if(!Value::is_matching<Type::LIST_BYTES>(fields_ptr, &vfield, &more))
          return false;
        break;
      }

      default:
        return false;
    }
    if(!more)
      return true;
  }
  return false;
}

std::string Fields::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Fields::print(std::ostream& out) const {
  out << "SerialFields(size=" << fields.size() << " [";
  if(!fields.empty()) for(size_t i = 0; ; ) {
    fields[i]->print(out << '(');
    out << ')';
    if(++i != fields.size())
      out << ',';
    else
      break;
  }
  out << "])";
}
//



}}}}}

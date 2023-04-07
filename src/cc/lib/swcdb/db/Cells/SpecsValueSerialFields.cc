/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/SpecsValueSerialFields.h"


namespace SWC { namespace DB { namespace Specs {
namespace Serial { namespace Value {



// Field INT64
Field_INT64::Field_INT64(uint24_t a_fid,
                         Condition::Comp a_comp, int64_t a_value)
                        : Field(a_fid), comp(a_comp), value(a_value) {
}

Field_INT64::Field_INT64(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          value(Serialization::decode_vi64(bufp, remainp)) {
}

size_t Field_INT64::encoded_length() const noexcept {
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
      << Condition::to_string(comp) << value;
}
//



// Field DOUBLE
Field_DOUBLE::Field_DOUBLE(uint24_t a_fid, Condition::Comp a_comp,
                           const long double& a_value)
                          : Field(a_fid), comp(a_comp), value(a_value) {
}

Field_DOUBLE::Field_DOUBLE(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          value(Serialization::decode_double(bufp, remainp)) {
}

size_t Field_DOUBLE::encoded_length() const noexcept {
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
      << Condition::to_string(comp) << value;
}
//



// Field BYTES
Field_BYTES::Field_BYTES(uint24_t a_fid, Condition::Comp a_comp,
                         const uint8_t* ptr, size_t len,
                         bool take_ownership)
                        : Field(a_fid), comp(a_comp), value() {
  take_ownership
    ? value.assign(ptr, len)
    : value.set(const_cast<uint8_t*>(ptr), len, false);
}

Field_BYTES::Field_BYTES(const uint8_t** bufp, size_t* remainp,
                         bool take_ownership)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          value() {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  take_ownership
    ? value.assign(ptr, len)
    : value.set(const_cast<uint8_t*>(ptr), len, false);
}

size_t Field_BYTES::encoded_length() const noexcept {
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
      << Condition::to_string(comp) << '"';
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

Field_KEY::Field_KEY(uint24_t a_fid, Types::KeySeq a_seq)
                    : Field(a_fid), seq(a_seq), key() {
}

Field_KEY::Field_KEY(uint24_t a_fid, Types::KeySeq a_seq, const Key& a_key)
                    : Field(a_fid), seq(a_seq), key(a_key) {
}

Field_KEY::Field_KEY(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          seq(Types::KeySeq(Serialization::decode_i8(bufp, remainp))),
          key(bufp, remainp) {
}

size_t Field_KEY::encoded_length() const noexcept {
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
Field_LIST_INT64::Field_LIST_INT64(uint24_t a_fid, Condition::Comp a_comp)
                                  : Field(a_fid), comp(a_comp),
                                    items(), _found() {
}

Field_LIST_INT64::Field_LIST_INT64(uint24_t a_fid, Condition::Comp a_comp,
                                   const Field_LIST_INT64::Vec& a_items)
                                  : Field(a_fid),
                                    comp(a_comp), items(a_items), _found() {
}

Field_LIST_INT64::Field_LIST_INT64(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          items(), _found() {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  while(len) {
    auto vcomp = Condition::Comp(Serialization::decode_i8(&ptr, &len));
    items.emplace_back(vcomp, Serialization::decode_vi64(&ptr, &len));
  }
  if(comp == Condition::SPS || comp == Condition::SBS)
    _found.resize(items.size());
}

size_t Field_LIST_INT64::encoded_length() const noexcept {
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
      auto it = items.cbegin();
      for(; remain && it != items.cend(); ++it) {
        if(!Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain))))
          return true;
      }
      return remain || it != items.cend();
    }

    case Condition::GT:
    case Condition::LT:
    case Condition::GE:
    case Condition::LE:
    case Condition::EQ: {
      auto it = items.cbegin();
      for(; remain && it != items.cend(); ++it) {
        if(!Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain))))
          break;
      }
      return remain
        ? it == items.cend() &&
          (comp == Condition::GT || comp == Condition::GE)
        : (it == items.cend()
            ? (comp == Condition::EQ ||
               comp == Condition::LE || comp == Condition::GE)
            : (comp == Condition::LT || comp == Condition::LE));
    }

    case Condition::SBS: {
      uint32_t sz = items.size();
      if(!sz)
        return true;
      if(!remain)
        return false;
      uint32_t count = sz;
      for(auto& f : _found)
        f = false;
      while(remain) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(uint32_t i = 0; i < sz; ++i) {
          if(!_found[i] &&
             Condition::is_matching(items[i].comp, items[i].value, v)) {
            if(!--count)
              return true;
            _found[i] = true;
            break;
          }
        }
      }
      return false;
    }

    case Condition::SPS: {
      if(!remain)
        return true;
      uint32_t sz = items.size();
      if(!sz)
        return false;
      for(auto& f : _found)
        f = false;
      while(remain) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(uint32_t i = 0; ;) {
          if(!_found[i] &&
             Condition::is_matching(items[i].comp, items[i].value, v)) {
            _found[i] = true;
            break;
          }
          if(++i == sz)
            return false;
        }
      }
      return true;
    }

    case Condition::POSBS: {
      auto it = items.cbegin();
      for(; remain && it != items.cend(); ) {
        if(Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain))))
          ++it;
      }
      return it == items.cend();
    }

    case Condition::FOSBS: {
      auto it = items.cbegin();
      for(bool start = false; remain && it != items.cend(); ) {
        if(Condition::is_matching(
              it->comp, it->value,
              int64_t(Serialization::decode_vi64(&ptr, &remain)))) {
          start = true;
          ++it;
        } else if(start) {
          return false;
        }
      }
      return it == items.cend();
    }

    case Condition::POSPS: {
      for(auto ord = items.cbegin(); remain && ord != items.cend(); ) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(auto it = ord; ;) {
          if(Condition::is_matching(it->comp, it->value, v)) {
            ++ord;
            break;
          }
          if(++it == items.cend())
            return false;
        }
      }
      return !remain;
    }

    case Condition::FOSPS: {
      bool start = false;
      for(auto ord = items.cbegin(); remain && ord != items.cend(); ) {
        int64_t v = Serialization::decode_vi64(&ptr, &remain);
        for(auto it = ord; ;) {
          if(Condition::is_matching(it->comp, it->value, v)) {
            start = true;
            ord = ++it;
            break;
          } else if(start) {
            return false;
          }
          if(++it == items.cend())
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
  out << fid << ":LI:" << Condition::to_string(comp) << '[';
  if(!items.empty()) for(auto it = items.cbegin(); ;) {
    out << Condition::to_string(it->comp) << it->value;
    if(++it == items.cend())
      break;
    out << ',';
  }
  out << ']';
}
//



// Field LIST_BYTES
Field_LIST_BYTES::Field_LIST_BYTES(uint24_t a_fid, Condition::Comp a_comp)
                                  : Field(a_fid), comp(a_comp),
                                    items(), _found() {
}

Field_LIST_BYTES::Field_LIST_BYTES(uint24_t a_fid, Condition::Comp a_comp,
                                   const Field_LIST_BYTES::Vec& a_items)
                                  : Field(a_fid),
                                    comp(a_comp), items(a_items), _found() {
}

Field_LIST_BYTES::Field_LIST_BYTES(const uint8_t** bufp, size_t* remainp)
        : Field(bufp, remainp),
          comp(Condition::Comp(Serialization::decode_i8(bufp, remainp))),
          items(), _found() {
  size_t len;
  const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
  while(len) {
    auto& item = items.emplace_back();
    item.comp = Condition::Comp(Serialization::decode_i8(&ptr, &len));
    size_t vlen;
    const uint8_t* vptr = Serialization::decode_bytes(&ptr, &len, &vlen);
    item.value.append(reinterpret_cast<const char*>(vptr), vlen);
  }
  if(comp == Condition::SPS || comp == Condition::SBS)
    _found.resize(items.size());
}

size_t Field_LIST_BYTES::encoded_length() const noexcept {
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
      auto it = items.cbegin();
      for(; remain && it != items.cend(); ++it) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        if(!Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->value.c_str()),
              it->value.size(),
              vptr, vlen))
          return true;
      }
      return remain || it != items.cend();
    }

    case Condition::GT:
    case Condition::LT:
    case Condition::GE:
    case Condition::LE:
    case Condition::EQ: {
      auto it = items.cbegin();
      for(; remain && it != items.cend(); ++it) {
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
        ? it == items.cend() &&
          (comp == Condition::GT || comp == Condition::GE)
        : (it == items.cend()
            ? (comp == Condition::EQ ||
               comp == Condition::LE || comp == Condition::GE)
            : (comp == Condition::LT || comp == Condition::LE));
    }

    case Condition::SBS: {
      uint32_t sz = items.size();
      if(!sz)
        return true;
      if(!remain)
        return false;
      uint32_t count = sz;
      for(auto& f : _found)
        f = false;
      while(remain) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        for(uint32_t i = 0; i < sz; ++i) {
          if(!_found[i] &&
             Condition::is_matching_extended(
              items[i].comp,
              reinterpret_cast<const uint8_t*>(items[i].value.c_str()),
              items[i].value.size(),
              vptr, vlen)) {
            if(!--count)
              return true;
            _found[i] = true;
            break;
          }
        }
      }
      return false;
    }

    case Condition::SPS: {
      if(!remain)
        return true;
      uint32_t sz = items.size();
      if(!sz)
        return false;
      for(auto& f : _found)
        f = false;
      while(remain) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(
          &ptr, &remain, &vlen);
        for(uint32_t i = 0; ;) {
          if(!_found[i] &&
             Condition::is_matching_extended(
              items[i].comp,
              reinterpret_cast<const uint8_t*>(items[i].value.c_str()),
              items[i].value.size(),
              vptr, vlen)) {
            _found[i] = true;
            break;
          }
          if(++i == sz)
            return false;
        }
      }
      return true;
    }

    case Condition::POSBS: {
      auto it = items.cbegin();
      for(; remain && it != items.cend(); ) {
        size_t vlen;
        const uint8_t* vptr = Serialization::decode_bytes(&ptr, &remain, &vlen);
        if(Condition::is_matching_extended(
            it->comp,
            reinterpret_cast<const uint8_t*>(it->value.c_str()),
            it->value.size(),
            vptr, vlen))
          ++it;
      }
      return it == items.cend();
    }

    case Condition::FOSBS: {
      auto it = items.cbegin();
      for(bool start = false; remain && it != items.cend(); ) {
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
      return it == items.cend();
    }

    case Condition::POSPS: {
      for(auto ord = items.cbegin(); remain && ord != items.cend(); ) {
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
          if(++it == items.cend())
            return false;
        }
      }
      return !remain;
    }

    case Condition::FOSPS: {
      bool start = false;
      for(auto ord = items.cbegin(); remain && ord != items.cend(); ) {
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
          if(++it == items.cend())
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
  out << fid << ":LB:" << Condition::to_string(comp) << '[';
  if(!items.empty()) for(auto it = items.cbegin(); ;) {
    out << Condition::to_string(it->comp) << '\'';
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
Fields::Fields(const uint8_t* ptr, size_t len, bool own)
              : fields(), _fields_ptr() {
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
        fields.emplace_back(new Field_BYTES(&ptr, &len, own));
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
  _fields_ptr.resize(fields.size());
}

bool Fields::has_field_id(uint24_t fid) const noexcept {
  for(auto& field : fields) {
    if(field->fid == fid)
      return true;
  }
  return false;
}

void Fields::add(Field::Ptr&& field) {
  fields.push_back(std::move(field));
}

size_t Fields::encoded_length() const noexcept {
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
is_matching(Core::Vector<Field*>& fields_ptr,
            Cell::Serial::Value::Field* vfieldp,
            bool* more) {
  for(auto it = fields_ptr.begin(); it != fields_ptr.cend(); ++it) {
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

  StaticBuffer v;
  cell.get_value(v);
  if(!v.size)
    return false;

  const uint8_t* ptr = v.base;
  size_t remain = v.size;
  for(size_t i = 0; i < fields.size(); ++i)
    _fields_ptr[i] = fields[i].get();

  while(remain) {

    bool more = false;
    switch(Cell::Serial::Value::read_type(&ptr, &remain)) {

      case Type::INT64: {
        Cell::Serial::Value::Field_INT64 vfield(&ptr, &remain);
        if(!Value::is_matching<Type::INT64>(_fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::DOUBLE: {
        Cell::Serial::Value::Field_DOUBLE vfield(&ptr, &remain);
        if(!Value::is_matching<Type::DOUBLE>(_fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::BYTES: {
        Cell::Serial::Value::Field_BYTES vfield(&ptr, &remain);
        if(!Value::is_matching<Type::BYTES>(_fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::KEY: {
        Cell::Serial::Value::Field_KEY vfield(&ptr, &remain);
        if(!Value::is_matching<Type::KEY>(_fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::LIST_INT64: {
        Cell::Serial::Value::Field_LIST_INT64 vfield(&ptr, &remain);
        if(!Value::is_matching<Type::LIST_INT64>(_fields_ptr, &vfield, &more))
          return false;
        break;
      }

      case Type::LIST_BYTES: {
        Cell::Serial::Value::Field_LIST_BYTES vfield(&ptr, &remain);
        if(!Value::is_matching<Type::LIST_BYTES>(_fields_ptr, &vfield, &more))
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

/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */



#include "swcdb/db/Cells/SpecsFlags.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


Flags::Flags(): limit(0), offset(0), max_versions(0), 
                options(0), was_set(false) {
}

Flags::Flags(const Flags &other) {
  copy(other);
}

void Flags::copy(const Flags &other) {
  limit           = other.limit;
  offset          = other.offset;
  max_versions    = other.max_versions;
  options         = other.options;
  was_set         = other.was_set;
}

Flags::~Flags() { }

const bool Flags::is_only_keys() const {
  return options & ONLY_KEYS;
}

const bool Flags::is_only_deletes() const {
  return options & ONLY_DELETES;
}

void Flags::set_only_keys() {
  options |= ONLY_KEYS;
}

void Flags::set_only_deletes() {
  options |= ONLY_DELETES;
}

const bool Flags::equal(const Flags &other) const {
  return  limit == other.limit && 
          offset == other.offset  && 
          max_versions == other.max_versions  && 
          options == other.options  && 
          was_set == other.was_set 
          ;
}

const size_t Flags::encoded_length() const {
  return Serialization::encoded_length_vi64(limit)
        +Serialization::encoded_length_vi64(offset)
        +Serialization::encoded_length_vi32(max_versions)
        +1;
}

void Flags::encode(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, limit);
  Serialization::encode_vi64(bufp, offset);
  Serialization::encode_vi32(bufp, max_versions);
  Serialization::encode_i8(bufp, options);
}

void Flags::decode(const uint8_t **bufp, size_t *remainp){
  limit = Serialization::decode_vi64(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
  max_versions = Serialization::decode_vi32(bufp, remainp);
  options = Serialization::decode_i8(bufp, remainp);
}

const std::string Flags::to_string() const {
  std::string s("Flags(");
  
  s.append("limit=");
  s.append(std::to_string(limit));
  s.append(" offset=");
  s.append(std::to_string(offset));

  s.append(" max_versions=");
  s.append(std::to_string(max_versions));

  s.append(" only_deletes=");
  s.append(std::to_string(is_only_deletes()));
  s.append(" only_keys=");
  s.append(std::to_string(is_only_keys()));
  s.append(" was_set=");
  s.append(was_set? "TRUE" : "FALSE");
  
  return s;
}

void Flags::display(std::ostream& out) const {
  out << "limit=" << limit  << " offset=" << offset  
      << " max_versions=" << max_versions 
      << " only_deletes=" << is_only_deletes() 
      << " only_keys=" << is_only_keys()
      << " was_set=" << (was_set? "TRUE" : "FALSE")
      ; 
}


}}}

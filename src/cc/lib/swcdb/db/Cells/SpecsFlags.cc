/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/SpecsFlags.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


Flags::Flags(): limit(0), offset(0),
                max_versions(0), max_buffer(0), 
                options(0), was_set(false) {
}

void Flags::copy(const Flags &other) {
  limit           = other.limit;
  offset          = other.offset;
  max_versions    = other.max_versions;
  max_buffer      = other.max_buffer;
  options         = other.options;
  was_set         = other.was_set;
}

Flags::~Flags() { }

bool Flags::is_only_keys() const {
  return options & ONLY_KEYS;
}

bool Flags::is_only_deletes() const {
  return options & ONLY_DELETES;
}

void Flags::set_only_keys() {
  options |= ONLY_KEYS;
}

void Flags::set_only_deletes() {
  options |= ONLY_DELETES;
}

bool Flags::equal(const Flags &other) const {
  return  limit == other.limit && 
          offset == other.offset  && 
          max_versions == other.max_versions  && 
          max_buffer == other.max_buffer  && 
          options == other.options  && 
          was_set == other.was_set 
          ;
}

size_t Flags::encoded_length() const {
  return  Serialization::encoded_length_vi64(limit)
        + Serialization::encoded_length_vi64(offset)
        + Serialization::encoded_length_vi32(max_versions)
        + Serialization::encoded_length_vi32(max_buffer)
        + 1;
}

void Flags::encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, limit);
  Serialization::encode_vi64(bufp, offset);
  Serialization::encode_vi32(bufp, max_versions);
  Serialization::encode_vi32(bufp, max_buffer);
  Serialization::encode_i8(bufp, options);
}

void Flags::decode(const uint8_t** bufp, size_t* remainp){
  limit = Serialization::decode_vi64(bufp, remainp);
  offset = Serialization::decode_vi64(bufp, remainp);
  max_versions = Serialization::decode_vi32(bufp, remainp);
  max_buffer = Serialization::decode_vi32(bufp, remainp);
  options = Serialization::decode_i8(bufp, remainp);
}

std::string Flags::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Flags::print(std::ostream& out) const {
  display(out << "Flags(");
  out << ')';
}

void Flags::display(std::ostream& out) const {
  out << "was_set=" << (was_set? "TRUE" : "FALSE"); 
  if(limit)
    out << " limit=" << limit;
  if(offset)
    out << " offset=" << offset;
  if(max_versions)
     out << " max_versions=" << max_versions;
  if(max_buffer)
    out << " max_buffer=" << max_buffer;
  if(is_only_deletes())
    out << " only_deletes=true";
  if(is_only_keys())
    out << " only_keys=true";
}


}}}

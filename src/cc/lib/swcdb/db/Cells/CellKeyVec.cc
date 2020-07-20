/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Cells/CellKeyVec.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cell {

KeyVec::KeyVec() { }

KeyVec::~KeyVec() { }

void KeyVec::free() {
  clear();
}

size_t KeyVec::size_of_internal() const {
  size_t sz = 0;
  for(auto& f : *this) {
    sz += sizeof(f);
    sz += f.length();
  }
  return sz;
}

void KeyVec::copy(const KeyVec &other) {
  free();
  assign(other.cbegin(), other.cend());
}

SWC_SHOULD_INLINE
bool KeyVec::equal(const KeyVec &other) const {
  return *this == other;
}

SWC_SHOULD_INLINE
void KeyVec::add(const KeyVec::Fraction& fraction) {
  add(fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void KeyVec::add(const std::string& fraction) {
  add(fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void KeyVec::add(const char* fraction) {
  add(fraction, strlen(fraction));
}

SWC_SHOULD_INLINE
void KeyVec::add(const char* fraction, const uint32_t len) {
  add((const uint8_t*)fraction, len);
}

SWC_SHOULD_INLINE
void KeyVec::add(const uint8_t* fraction, const uint32_t len) {
  emplace_back(fraction, len);
}

SWC_SHOULD_INLINE
void KeyVec::insert(const uint32_t idx, const KeyVec::Fraction& fraction) {
  insert(idx, fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void KeyVec::insert(const uint32_t idx, const std::string& fraction) {
  insert(idx, fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void KeyVec::insert(const uint32_t idx, const char* fraction) {
  insert(idx, fraction, strlen(fraction));
}

SWC_SHOULD_INLINE
void KeyVec::insert(const uint32_t idx, const char* fraction, 
                    const uint32_t len) {
  insert(idx, (const uint8_t*)fraction, len);
}

SWC_SHOULD_INLINE
void KeyVec::insert(const uint32_t idx, const uint8_t* fraction, 
                    const uint32_t len) {
  emplace(cbegin() + idx, fraction, len);
}

SWC_SHOULD_INLINE
void KeyVec::set(const uint32_t idx, const KeyVec::Fraction& fraction) {
  set(idx, fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void KeyVec::set(const uint32_t idx, const std::string& fraction) {
  set(idx, fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void KeyVec::set(const uint32_t idx, const char* fraction) {
  set(idx, fraction, strlen(fraction));
}

SWC_SHOULD_INLINE
void KeyVec::set(const uint32_t idx, const char* fraction, 
                 const uint32_t len) {
  set(idx, (const uint8_t*)fraction, len);
}

SWC_SHOULD_INLINE
void KeyVec::set(const uint32_t idx, const uint8_t* fraction, 
                 const uint32_t len) {
  (*this)[idx].assign(fraction, len);
}

void KeyVec::remove(const uint32_t idx) {
  if(idx >= size())
    return;
  erase(cbegin()+idx);
}


SWC_SHOULD_INLINE
const KeyVec::Fraction& KeyVec::get(const uint32_t idx) const {
  return (*this)[idx];
}

SWC_SHOULD_INLINE
void KeyVec::get(const uint32_t idx, KeyVec::Fraction& fraction) const {
  fraction = (*this)[idx];
}

SWC_SHOULD_INLINE
void KeyVec::get(const uint32_t idx, std::string& fraction) const {
  fraction.assign((const char*)(*this)[idx].data(), (*this)[idx].size());
}

uint32_t KeyVec::encoded_length() const {
  uint32_t len = Serialization::encoded_length_vi32(size());
  for(auto it = cbegin(); it < cend(); ++it)
    len += Serialization::encoded_length_vi32(it->length()) + it->length();
  return len;
}

void KeyVec::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, size());
  uint32_t len;
  for(auto it = cbegin(); it < cend(); ++it) {
    Serialization::encode_vi32(bufp, len = it->length());
    memcpy(*bufp, it->data(), len);
    *bufp += len;
  }
}

void KeyVec::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  resize(Serialization::decode_vi32(bufp, remainp));
  uint32_t len;
  for(auto it = begin(); it < end(); ++it) {
    len = Serialization::decode_vi32(bufp);
    it->assign(*bufp, len);
    *bufp += len;
  }
}

std::string KeyVec::to_string() const {
  std::string s("Key(");
  s.append("sz=");
  s.append(std::to_string(size()));
  s.append(" fractions=[");
  char hex[5];
  for(auto it = cbegin(); it < cend(); ) {
    s += '"';
    for(auto chrp = it->cbegin(); chrp < it->cend(); ++chrp) {
      if(*chrp == '"')
        s += '\\';
      if(31 < *chrp && *chrp < 127) {
        s += *chrp;
      } else {
        sprintf(hex, "0x%X", *chrp);
        s.append(hex, 4);
      }
    }
    s += '"';
    if(++it < cend())
      s.append(", ");
  }
  s.append("])");
  return s;
}


}}}

/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */



#include "swcdb/db/Cells/SpecsKey.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {
  


bool Fraction::operator==(const Fraction &other) const {
  return other.comp == comp && length() == other.length() && 
         memcmp(data(), other.data(), length()) == 0;
}

uint32_t Fraction::encoded_length() const {
  return 1 + Serialization::encoded_length_vi32(size()) + size();
}

void Fraction::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  Serialization::encode_vi32(bufp, size());
  if(!empty()) {
    memcpy(*bufp, data(), size());
    *bufp += size();
  }
}

void Fraction::decode(const uint8_t **bufp, size_t* remainp) {
  clear();
  comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
  if(uint32_t len = Serialization::decode_vi32(bufp, remainp)) {
    append((const char*)*bufp, len);
    *bufp += len;
    *remainp -= len;
  }
}



Key::Key() { }

Key::~Key() { }

Key::Key(const DB::Cell::Key &cell_key, Condition::Comp comp) {
  set(cell_key, comp);
}

void Key::free() {
  clear();
}

void Key::copy(const Key &other) {
  clear();
  assign(other.begin(), other.end());
}

bool Key::equal(const Key &other) const {
  return *this == other;
}

void Key::set(const DB::Cell::Key &cell_key, Condition::Comp comp) {
  clear();
  resize(cell_key.count);
  
  uint32_t len;
  const uint8_t* ptr = cell_key.data;
  for(auto it=begin(); it < end(); ++it) {
    it->comp = comp;
    if(len = Serialization::decode_vi32(&ptr)) {
      it->append((const char*)ptr, len);
      ptr += len;
    }
  }
}

void Key::set(int32_t idx, Condition::Comp comp) {
  if(empty())
    return;
  (begin()+idx)->comp = comp;
}

void Key::add(const char* buf, uint32_t len, Condition::Comp comp) {
  Fraction& f = emplace_back();
  f.comp = comp;
  if(len)
    f.append(buf, len);
}

void Key::add(const std::string& fraction, Condition::Comp comp) {
  add(fraction.data(), fraction.length(), comp);
}

void Key::add(const std::string_view& fraction, Condition::Comp comp) {
  add(fraction.data(), fraction.length(), comp);
}

void Key::add(const char* fraction, Condition::Comp comp) {
  add(fraction, strlen(fraction), comp);
}

void Key::add(const uint8_t* fraction, uint32_t len, Condition::Comp comp) {
  add((const char*)fraction, len, comp);
}


void Key::insert(uint32_t idx, const char* buf, uint32_t len, 
                 Condition::Comp comp) {
  auto it = emplace(begin() + idx);
  it->comp = comp;
  if(len)
    it->append(buf, len);
}

void Key::insert(uint32_t idx, const std::string& fraction, 
                 Condition::Comp comp) {
  insert(idx, fraction.data(), fraction.length(), comp);
}

void Key::insert(uint32_t idx, const std::string_view& fraction, 
                 Condition::Comp comp) {
  insert(idx, fraction.data(), fraction.length(), comp);
}

void Key::insert(uint32_t idx, const uint8_t* fraction, uint32_t len,
                 Condition::Comp comp) {
  insert(idx, (const char*)fraction, len, comp);
}

void Key::insert(uint32_t idx, const char* fraction, Condition::Comp comp) {
  insert(idx, fraction, strlen(fraction), comp);
}


std::string_view Key::get(const uint32_t idx, Condition::Comp& comp) const {
  auto& f = (*this)[idx];
  comp = f.comp;
  return f;
}

std::string_view Key::get(const uint32_t idx) const {
  return (*this)[idx];
}

void Key::get(DB::Cell::Key& key) const {
  key.free();
  if(!empty()) 
    for(auto it=begin(); it < end(); ++it)
      key.add(*it);
}

void Key::remove(uint32_t idx, bool recursive) {
  if(recursive)
    erase(begin()+idx, end());
  else
    erase(begin()+idx);
}

uint32_t Key::encoded_length() const {
  uint32_t len = Serialization::encoded_length_vi32(size());
  for(auto it = begin(); it < end(); ++it)
    len += it->encoded_length();
  return len;
}

void Key::encode(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, size());
  for(auto it = begin(); it < end(); ++it) 
    it->encode(bufp);
}

void Key::decode(const uint8_t **bufp, size_t* remainp, bool owner) {
  clear();
  resize(Serialization::decode_vi32(bufp, remainp));
  for(auto it = begin(); it < end(); ++it)
    it->decode(bufp, remainp);
}

std::string Key::to_string() const {
  std::string s("Key(");
  std::stringstream ss;
  display(ss);
  s.append(ss.str());
  s.append(")");
  return s;
}

void Key::display(std::ostream& out, bool pretty) const {
  out << "size=" << size() << " fractions=[";
  char hex[5];
  hex[4] = 0;
  for(auto it = begin(); it < end(); ) {
    out << Condition::to_string(it->comp) 
        << '"';
    if(pretty) {
      for(auto chrp = it->cbegin(); chrp < it->cend(); ++chrp) {
        if(31 < (uint8_t)*chrp && (uint8_t)*chrp < 127) {
          out << *chrp;
        } else {
          sprintf(hex, "0x%X", (uint8_t)*chrp);
          out << hex;
        }
      }
    } else {
      out << *it;
    }
    out << '"';
    if(++it < end())
      out << ", "; 
  }
  out << "]"; 
}


}}}

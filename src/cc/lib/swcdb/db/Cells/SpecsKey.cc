/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */



#include "swcdb/db/Cells/SpecsKey.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {
  


bool Fraction::operator==(const Fraction &other) const {
  return other.comp == comp && value.length() == other.value.length() && 
         memcmp(value.data(), other.value.data(), value.length()) == 0;
}

const uint32_t Fraction::encoded_length() const {
  return 1 + Serialization::encoded_length_vi32(value.size()) + value.size();
}

void Fraction::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  Serialization::encode_vi32(bufp, value.size());
  memcpy(*bufp, value.data(), value.size());
  *bufp += value.size();
}

void Fraction::decode(const uint8_t **bufp, size_t* remainp) {
  value.clear();
  comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
  uint32_t len = Serialization::decode_vi32(bufp, remainp);
  value.append((const char*)*bufp, len);
  *bufp += len;
  *remainp -= len;
}



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

const bool Key::equal(const Key &other) const {
  return *this == other;
}

void Key::set(const DB::Cell::Key &cell_key, Condition::Comp comp) {
  clear();
  resize(cell_key.count);
  
  uint32_t len;
  const uint8_t* ptr = cell_key.data;
  for(auto it=begin(); it < end(); ++it) {
    it->comp = comp;
    len = Serialization::decode_vi32(&ptr);
    it->value.append((const char*)ptr, len);
    ptr += len;
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
  f.value.append(buf, len);
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
  auto it = emplace(begin()+idx);
  it->comp = comp;
  it->value.append(buf, len);
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


const std::string_view Key::get(const uint32_t idx, 
                                Condition::Comp& comp) const {
  auto& f = (*this)[idx];
  comp = f.comp;
  return f.value;
}

const std::string_view Key::get(const uint32_t idx) const {
  return (*this)[idx].value;
}

void Key::get(DB::Cell::Key& key) const {
  key.free();
  if(!empty()) 
    for(auto it=begin(); it < end(); ++it)
      key.add(it->value);
}

void Key::remove(uint32_t idx, bool recursive) {
  if(recursive)
    erase(begin()+idx, end());
  else
    erase(begin()+idx);
}

const bool Key::is_matching(const DB::Cell::Key &other) const {
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = other.data;
  uint32_t c = other.count;
  uint32_t len;

  for(auto it = begin(); c && it < end(); ++it, --c) {
    len = Serialization::decode_vi32(&ptr);
    if(!Condition::is_matching(
        comp = it->comp, 
        (const uint8_t*)it->value.data(), it->value.size(),
        ptr, len
        ))
      return false;
    ptr += len;
  }

  if(size() == other.count) 
    return true;

  switch(comp) {
    case Condition::LT:
    case Condition::LE:
      return empty() || size() > other.count;
    case Condition::GT:
    case Condition::GE:
      return empty() || size() < other.count;
    case Condition::PF:
    case Condition::RE:
      return size() < other.count;
    case Condition::NE:
    case Condition::NONE:
      return true;
    default: // Condition::EQ:
      return false;
  }
}

const uint32_t Key::encoded_length() const {
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

const std::string Key::to_string() const {
  std::string s("Key(size=");
  s.append(std::to_string(size()));
  s.append(" fractions=[");

  for(auto it = begin(); it < end();) {
    s.append(Condition::to_string(it->comp));
    s.append("(");
    s.append(it->value);
    s.append(")");
    if(++it < end())
      s.append(",");
  }
  s.append("])");
  return s;
}

void Key::display(std::ostream& out) const {
  out << "size=" << size() << " fractions=[";
  for(auto it = begin(); it < end();) {
    out << Condition::to_string(it->comp);
    out << '"' << it->value << '"';
    if(++it < end())
      out << ", "; 
  }
  out << "]"; 
}


}}}

/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cell {


Key::Key(bool own): own(own), count(0), size(0), data(0) { }

Key::Key(const Key& other)
        : own(other.size), count(other.count), size(other.size),
          data(_data(other.data)) {
}

void Key::copy(const Key& other) {
  free(); 
  own = true;
  size = other.size;
  count = other.count;
  data = _data(other.data);
}

Key::~Key() {
  if(own && data)
    delete [] data;
}

void Key::free() {
  if(own && data)
    delete [] data;
  data = 0;
  size = 0;
  count = 0;
}

bool Key::sane() const {
  return (count && size && data) || (!count && !size && !data);
}

void Key::add(const std::string_view& fraction) {
  add((const uint8_t*)fraction.data(), fraction.length());
}

void Key::add(const std::string& fraction) {
  add((const uint8_t*)fraction.data(), fraction.length());
}

void Key::add(const char* fraction) {
  add((const uint8_t*)fraction, strlen(fraction));
}

void Key::add(const char* fraction, uint32_t len) {
  add((const uint8_t*)fraction, len);
}

void Key::add(const uint8_t* fraction, uint32_t len) {
  const uint8_t* old = data;
  uint32_t old_size = size;

  uint8_t* ptr = data = 
    new uint8_t[size += Serialization::encoded_length_vi32(len) + len];
  if(old) {
    memcpy(ptr, old, old_size);
    ptr += old_size;
    if(own)
      delete [] old;
  }
  Serialization::encode_vi32(&ptr, len);
  memcpy(ptr, fraction, len);
  ++count;
  own = true;
}

void Key::insert(uint32_t idx, const std::string& fraction) {
  insert(idx, (const uint8_t*)fraction.data(), fraction.length());
}

void Key::insert(uint32_t idx, const char* fraction) {
  insert(idx, (const uint8_t*)fraction, strlen(fraction));
}

void Key::insert(uint32_t idx, const char* fraction, uint32_t len) {
  insert(idx, (const uint8_t*)fraction, len);
}

void Key::insert(uint32_t idx, const uint8_t* fraction, uint32_t len) {
  if(!data || idx >= count) {
    add(fraction, len);
    return;
  }

  uint32_t prev_size = size;
  uint32_t f_size = Serialization::encoded_length_vi32(len) + len;
  size += f_size;

  uint8_t* data_tmp = new uint8_t[size]; 
  const uint8_t* ptr_tmp = data;
 
  uint8_t* fraction_ptr;
  uint32_t pos = 0;
  uint32_t offset = 0;

  for(;;) {
    if(idx == pos++) {
      if(offset) 
        memcpy(data_tmp, data, offset);
      fraction_ptr = data_tmp + offset;
      Serialization::encode_vi32(&fraction_ptr, len);
      memcpy(fraction_ptr, fraction, len);
      fraction_ptr += len;
      break;
    }
    ptr_tmp += Serialization::decode_vi32(&ptr_tmp);
    offset += ptr_tmp-data;
  }
  
  if(prev_size-offset)
    memcpy(fraction_ptr, ptr_tmp, prev_size-offset);
  
  if(own)
    delete [] data;
  else
      own = true;
  data = data_tmp;
  ++count;
}

void Key::remove(uint32_t idx, bool recursive) {
  if(!data || idx >= count)
    return;

  const uint8_t* ptr_tmp = data;
  if(!own) {
    own = true;
    ptr_tmp = (data = _data(data));
  }

  uint8_t* begin;
  for(uint24_t offset = 0; offset < count; ++offset) {
    begin = (uint8_t*)ptr_tmp;
    ptr_tmp += Serialization::decode_vi32(&ptr_tmp);
    if(offset < idx) 
      continue;
    
    if(recursive) {
      count = offset;
      size = begin-data;
    } else if(--count) {
      memmove(begin, ptr_tmp, size-(ptr_tmp-data)); 
      size -= ptr_tmp-begin;
    }
    
    ptr_tmp = data;
    if(count) {
      data = _data(ptr_tmp);
    } else {
      data = 0;
      size = 0;
    }
    delete ptr_tmp;
    break;
  }
}

std::string Key::get_string(uint32_t idx) const {
  const char* fraction;
  uint32_t length;
  get(idx, &fraction, &length);
  return std::string(fraction, length);
}

void Key::get(uint32_t idx, const char** fraction, uint32_t* length) const {
  if(data && ++idx <= count) {
    const uint8_t* ptr = data;
    for(; idx ; --idx)
      ptr += (*length = Serialization::decode_vi32(&ptr));
    if(!idx) { 
      *fraction = (const char*)ptr - *length;
      return;
    }
  }
  *fraction = 0;
  *length = 0;
}

bool Key::equal(const Key& other) const {
  return count == other.count && 
        ((!data && !other.data) || 
         Condition::eq(data, size, other.data, other.size));
}

Condition::Comp Key::compare(const Key& other, uint32_t max, 
                             bool empty_ok, bool empty_eq) const {
  if(uint24_t min = count < other.count ? count : other.count) {
    if(max && min > max)
      min = max;  
    const uint8_t* p1 = data;
    const uint8_t* p2 = other.data;
    uint32_t sz1;
    uint32_t sz2;
    for(Condition::Comp comp; min; --min, p1 += sz1, p2 += sz2) {
      sz2 = Serialization::decode_vi32(&p2);
      if(!(sz1 = Serialization::decode_vi32(&p1)) && empty_ok) {
        if(empty_eq)
          return Condition::EQ;
        continue;
      }
      if((comp = Condition::condition(p1, sz1, p2, sz2)) != Condition::EQ)
        return comp;
    }
  }
  return count != other.count && (!max || max > count || max > other.count)
        ? count > other.count ? Condition::LT : Condition::GT
          : Condition::EQ;
}

bool Key::align(KeyVec& start, KeyVec& finish) const {
  const uint8_t* ptr = data;
  uint32_t len;
  bool chg = false;
  for(uint32_t c = 0; c < count; ++c, ptr += len) {
    len = Serialization::decode_vi32(&ptr);

    if(c == start.size()) {
      start.add(ptr, len);
      chg = true;
    } else if(Condition::condition(
                (const uint8_t*)start[c].data(), start[c].length(),
                ptr, len
              ) == Condition::LT) {
      start.set(c, ptr, len);
      chg = true;
    }

    if(c == finish.size()) {
      finish.add(ptr, len);
      chg = true;
    } else if(Condition::condition(
                (const uint8_t*)finish[c].data(), finish[c].length(),
                ptr, len
                ) == Condition::GT) {
      finish.set(c, ptr, len);
      chg = true;
    }
  }
  return chg;
}

bool Key::compare(const KeyVec& other, Condition::Comp break_if,
                  uint32_t max, bool empty_ok) const {
  const uint8_t* ptr = data;
  uint32_t len = 0;
  if(!max)
    max = count > other.size() ? (uint32_t)count : other.size();
  for(uint32_t c = 0; c<max; ++c, ptr += len) {

    if(c == count || c == other.size())
      return count > other.size() 
            ? break_if != Condition::LT 
            : break_if != Condition::GT;

    if(!(len = Serialization::decode_vi32(&ptr)) && empty_ok)
      continue;

    auto& r = other[c];
    if(Condition::condition(ptr, len, (const uint8_t*)r.data(), r.length())
                                                            == break_if)
      return false;
  }
  return true;
}

bool Key::empty() const {
  return !count;
}

uint32_t Key::encoded_length() const {
  return Serialization::encoded_length_vi24(count) + size;
}

void Key::encode(uint8_t **bufp) const {
  Serialization::encode_vi24(bufp, count);
  if(size) {
    memcpy(*bufp, data, size);
    *bufp += size;
  }
}

void Key::decode(const uint8_t **bufp, size_t* remainp, bool owner) {
  free();
  if(count = Serialization::decode_vi24(bufp, remainp)) {
    uint24_t n=count;
    const uint8_t* ptr_start = *bufp;
    do *bufp += Serialization::decode_vi32(bufp); 
    while(--n);
    *remainp -= (size = *bufp - ptr_start);
    data = (own = owner) ? _data(ptr_start) : (uint8_t*)ptr_start;
  }
}

void Key::convert_to(std::vector<std::string>& key) const {
  uint32_t len = 0;
  const uint8_t* ptr = data;
  key.clear();
  key.resize(count);
  for(auto it = key.begin(); it<key.end(); ++it, ptr+=len) {
    it->append((const char*)ptr, len = Serialization::decode_vi32(&ptr));
  }
}

void Key::read(const std::vector<std::string>& key)  {
  free();
  for(auto& f : key)
    add(f);
}

bool Key::equal(const std::vector<std::string>& key) const {
  if(key.size() != count)
    return false;
    
  uint32_t len;
  const uint8_t* ptr = data;
  for(auto it = key.begin(); it<key.end(); ++it, ptr+=len) {
    if(!Condition::eq(ptr, len = Serialization::decode_vi32(&ptr),
                      (const uint8_t*)it->data(), it->length()))
      return false;
  }
  return true;
}

std::string Key::to_string() const {
  std::string s("Key(");
  s.append("sz=");
  s.append(::SWC::to_string(count));
  s.append(" len=");
  s.append(std::to_string(size));
  s.append(" fractions=[");
  uint32_t len = 0;
  const uint8_t* ptr = data;
  for(uint24_t n=0; n<count; ++n,ptr+=len) {
    s.append("(");
    len = Serialization::decode_vi32(&ptr);
    s.append((const char*)ptr, len);
    s.append("),");
  }
  s.append("])");
  return s;
}

void Key::display(std::ostream& out, bool pretty) const {
  out << "["; 
  if(!count) {
    out << "]"; 
    return;
  }
    
  uint32_t len;
  const uint8_t* ptr = data;
  char hex[2];
  for(uint24_t n=0; n<count; ) {
    for(len = Serialization::decode_vi32(&ptr); len--; ++ptr) {
      if(pretty && (*ptr < 32 || *ptr > 126)) {
        sprintf(hex, "%X", *ptr);
        out << "0x" << hex;
      } else
          out << *ptr; 
    }
    if(++n < count)
      out << ", "; 
  }
  out << "]"; 
  
}

void Key::display_details(std::ostream& out, bool pretty) const {
  out << "size=" << size << " count=" << count << " fractions=[";
  uint32_t len;
  const uint8_t* ptr = data;
  char hex[2];
  for(uint24_t n=0; n<count; ) {
    out << '"';
    for(len = Serialization::decode_vi32(&ptr); len--; ++ptr) {
      if(pretty && (*ptr < 32 || *ptr > 126)) {
        sprintf(hex, "%X", *ptr);
        out << "0x" << hex;
      } else
          out << *ptr; 
    }
    out << '"';
    if(++n < count)
      out << ", "; 
  }
  out << "]"; 
}

uint8_t* Key::_data(const uint8_t* ptr) {
  return size ? (uint8_t*)memcpy(new uint8_t[size], ptr, size) : 0;
}


}}}

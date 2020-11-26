/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cell {


SWC_SHOULD_INLINE
Key::Key(bool own): own(own), count(0), size(0), data(0) { }

SWC_SHOULD_INLINE
Key::Key(const Key& other)
        : own(other.size), count(other.count), size(other.size),
          data(_data(other.data)) {
}

SWC_SHOULD_INLINE
Key::Key(const Key& other, bool own)
        : own(own), count(other.count), size(other.size),
          data(own ? _data(other.data): other.data) {
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

SWC_SHOULD_INLINE
void Key::add(const std::string_view& fraction) {
  add((const uint8_t*)fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void Key::add(const std::string& fraction) {
  add((const uint8_t*)fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void Key::add(const char* fraction) {
  add((const uint8_t*)fraction, strlen(fraction));
}

SWC_SHOULD_INLINE
void Key::add(const char* fraction, uint32_t len) {
  add((const uint8_t*)fraction, len);
}

void Key::add(const uint8_t* fraction, uint32_t len) {
  const uint8_t* old = data;
  uint32_t old_size = size;

  uint8_t* ptr = data = 
    new uint8_t[size += Serialization::encoded_length_vi24(len) + len];
  if(old) {
    memcpy(ptr, old, old_size);
    ptr += old_size;
    if(own)
      delete [] old;
  }
  Serialization::encode_vi24(&ptr, len);
  memcpy(ptr, fraction, len);
  ++count;
  own = true;
}

SWC_SHOULD_INLINE
void Key::add(const std::vector<std::string>& fractions) {  
  add(fractions.cbegin(), fractions.cend());
}

void Key::add(const std::vector<std::string>::const_iterator cbegin, 
              const std::vector<std::string>::const_iterator cend) {
  if(cbegin == cend)
    return;

  const uint8_t* old = data;
  uint32_t old_size = size;

  for(auto it=cbegin; it < cend; ++it)
    size += Serialization::encoded_length_vi24(it->size()) + it->size();
  
  uint8_t* ptr = data = new uint8_t[size];
  if(old) {
    memcpy(ptr, old, old_size);
    ptr += old_size;
    if(own)
      delete [] old;
  }
  for(auto it=cbegin; it < cend; ++it) {
    Serialization::encode_vi24(&ptr, it->size());
    memcpy(ptr, (const uint8_t*)it->data(), it->size());
    ptr += it->size();
  }
  count += cend - cbegin;
  own = true;
}

SWC_SHOULD_INLINE
void Key::insert(uint32_t idx, const std::string& fraction) {
  insert(idx, (const uint8_t*)fraction.data(), fraction.length());
}

SWC_SHOULD_INLINE
void Key::insert(uint32_t idx, const char* fraction) {
  insert(idx, (const uint8_t*)fraction, strlen(fraction));
}

SWC_SHOULD_INLINE
void Key::insert(uint32_t idx, const char* fraction, uint32_t len) {
  insert(idx, (const uint8_t*)fraction, len);
}

void Key::insert(uint32_t idx, const uint8_t* fraction, uint32_t len) {
  if(!data || idx >= count) {
    add(fraction, len);
    return;
  }

  uint32_t prev_size = size;
  uint32_t f_size = Serialization::encoded_length_vi24(len) + len;
  size += f_size;

  uint8_t* data_tmp = new uint8_t[size]; 
  const uint8_t* ptr_tmp = data;
 
  uint8_t* fraction_ptr;
  
  uint32_t offset = 0;
  for(uint32_t pos = 0;; ++pos) {
    if(idx == pos) {
      if(offset) 
        memcpy(data_tmp, data, offset);
      fraction_ptr = data_tmp + offset;
      Serialization::encode_vi24(&fraction_ptr, len);
      memcpy(fraction_ptr, fraction, len);
      fraction_ptr += len;
      break;
    }
    ptr_tmp += Serialization::decode_vi24(&ptr_tmp);
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
    ptr_tmp += Serialization::decode_vi24(&ptr_tmp);
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
  const char* fraction = 0;
  uint32_t length = 0;
  get(idx, &fraction, &length);
  if(!fraction)
    length = 0;
  return std::string(fraction, length);
}

void Key::get(uint32_t idx, const char** fraction, uint32_t* length) const {
  if(data && ++idx <= count) {
    const uint8_t* ptr = data;
    for(; idx ; --idx)
      ptr += (*length = Serialization::decode_vi24(&ptr));
    if(!idx) { 
      *fraction = (const char*)ptr - *length;
      return;
    }
  }
}

bool Key::equal(const Key& other) const {
  return count == other.count && 
        ((!data && !other.data) || 
         Condition::eq(data, size, other.data, other.size));
}

SWC_SHOULD_INLINE
bool Key::empty() const {
  return !count;
}

uint32_t Key::encoded_length() const {
  return Serialization::encoded_length_vi24(count) + size;
}

void Key::encode(uint8_t** bufp) const {
  Serialization::encode_vi24(bufp, count);
  if(size) {
    memcpy(*bufp, data, size);
    *bufp += size;
  }
}

void Key::decode(const uint8_t** bufp, size_t* remainp, bool owner) {
  free();
  if((count = Serialization::decode_vi24(bufp, remainp))) {
    uint24_t n=count;
    const uint8_t* ptr_start = *bufp;
    do *bufp += Serialization::decode_vi24(bufp); 
    while(--n);
    *remainp -= (size = *bufp - ptr_start);
    data = (own = owner) ? _data(ptr_start) : (uint8_t*)ptr_start;
  }
}

void Key::convert_to(std::vector<std::string>& key) const {
  uint24_t len;
  const uint8_t* ptr = data;
  key.clear();
  key.resize(count);
  for(auto it = key.begin(); it<key.end(); ++it, ptr+=len) {
    len = Serialization::decode_vi24(&ptr);
    it->assign((const char*)ptr, len);
  }
}

void Key::convert_to(std::vector<KeyVec::Fraction>& key) const {
  uint24_t len;
  const uint8_t* ptr = data;
  key.clear();
  key.resize(count);
  for(auto it = key.begin(); it<key.end(); ++it, ptr+=len) {
    len = Serialization::decode_vi24(&ptr);
    it->assign(ptr, len);
  }
}

void Key::read(const std::vector<std::string>& key)  {
  free();
  add(key.cbegin(), key.cend());
}

bool Key::equal(const std::vector<std::string>& key) const {
  if(key.size() != count)
    return false;
    
  uint24_t len;
  const uint8_t* ptr = data;
  for(auto it = key.begin(); it<key.end(); ++it, ptr+=len) {
    len = Serialization::decode_vi24(&ptr);
    if(!Condition::eq(ptr, len, (const uint8_t*)it->data(), it->length()))
      return false;
  }
  return true;
}

std::string Key::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Key::display_details(std::ostream& out, bool pretty) const {
  out << "size=" << size << " count=" << count << " fractions=";
  display(out, pretty);
}

void Key::display(std::ostream& out, bool pretty, const char* sep) const {
  out << '['; 
  if(!count) {
    out << ']'; 
    return;
  }
  uint24_t len;
  const uint8_t* ptr = data;
  char hex[5];
  hex[4] = 0;
  for(uint24_t n=0; n<count; ) {
    out << '"';
    for(len = Serialization::decode_vi24(&ptr); len; --len, ++ptr) {
      if(*ptr == '"')
        out << '\\';
      if(!pretty || (31 < *ptr && *ptr < 127)) {
        out << *ptr;
      } else {
        sprintf(hex, "0x%X", *ptr);
        out << hex;
      }
    }
    out << '"';
    if(++n < count)
      out << sep; 
  }
  out << ']'; 
  
}

void Key::print(std::ostream& out) const {
  out << "Key(";
  if(size)
    display_details(out, true);
  out << ')';
}

SWC_SHOULD_INLINE
uint8_t* Key::_data(const uint8_t* ptr) {
  return size ? (uint8_t*)memcpy(new uint8_t[size], ptr, size) : 0;
}


}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace DB { namespace Cell {



void Key::copy(uint24_t after_idx, const Key& other) {
  _free();
  own = true;
  if(other.data && ++after_idx < other.count) {
    count = other.count - after_idx;
    const uint8_t* ptr = other.data;
    for(; after_idx ; --after_idx)
      ptr += Serialization::decode_vi24(&ptr);
    size = other.size - (ptr - other.data);
    data = _data(ptr);
  } else {
    size = 0;
    count = 0;
    data = nullptr;
  }
}

void Key::add(const uint8_t* fraction, uint24_t len) {
  const uint8_t* old = data;
  uint32_t old_size = size;

  size += Serialization::encoded_length_vi24(len);
  size += uint32_t(len);
  uint8_t* ptr = data = new uint8_t[size];
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

SWC_SHOULD_INLINE
void Key::add(const std::vector<KeyVec::Fraction>& fractions) {
  add(fractions.cbegin(), fractions.cend());
}

template<typename T>
SWC_SHOULD_INLINE
void Key::add(const T cbegin,  const T cend) {
  if(cbegin == cend)
    return;

  const uint8_t* old = data;
  uint32_t old_size = size;

  for(auto it=cbegin; it != cend; ++it)
    size += Serialization::encoded_length_vi24(it->size()) + it->size();

  uint8_t* ptr = data = new uint8_t[size];
  if(old) {
    memcpy(ptr, old, old_size);
    ptr += old_size;
    if(own)
      delete [] old;
  }
  for(auto it=cbegin; it != cend; ++it) {
    Serialization::encode_vi24(&ptr, it->size());
    memcpy(ptr, reinterpret_cast<const uint8_t*>(it->c_str()), it->size());
    ptr += it->size();
  }
  count += cend - cbegin;
  own = true;
}

void Key::insert(uint32_t idx, const uint8_t* fraction, uint24_t len) {
  if(!data || idx >= count) {
    add(fraction, len);
    return;
  }

  uint32_t prev_size = size;
  size += Serialization::encoded_length_vi24(len);
  size += uint32_t(len);

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
    offset += ptr_tmp - data;
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
    begin = const_cast<uint8_t*>(ptr_tmp);
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
      data = nullptr;
      size = 0;
    }
    delete [] ptr_tmp;
    break;
  }
}

void Key::get(uint32_t idx, const char** fraction, uint32_t* length) const {
  if(data && idx < count) {
    const uint8_t* ptr = data;
    for(uint32_t len; ; --idx, ptr += len) {
      len = Serialization::decode_vi24(&ptr);
      if(!idx) {
        *fraction = reinterpret_cast<const char*>(ptr);
        *length = len;
        break;
      }
    }
  }
}

SWC_SHOULD_INLINE
void Key::convert_to(std::vector<std::string>& key) const {
  uint24_t len;
  const uint8_t* ptr = data;
  key.clear();
  key.resize(count);
  for(auto it = key.begin(); it != key.cend(); ++it, ptr+=len) {
    len = Serialization::decode_vi24(&ptr);
    it->assign(reinterpret_cast<const char*>(ptr), len);
  }
}

SWC_SHOULD_INLINE
void Key::convert_to(std::vector<KeyVec::Fraction>& key) const {
  uint24_t len;
  const uint8_t* ptr = data;
  key.clear();
  key.resize(count);
  for(auto it = key.begin(); it != key.cend(); ++it, ptr+=len) {
    len = Serialization::decode_vi24(&ptr);
    it->assign(ptr, len);
  }
}

SWC_SHOULD_INLINE
void Key::read(const std::vector<std::string>& key) {
  free();
  add(key.cbegin(), key.cend());
}

bool Key::equal(const std::vector<std::string>& key) const {
  if(key.size() != count)
    return false;

  uint24_t len;
  const uint8_t* ptr = data;
  for(auto it = key.cbegin(); it != key.cend(); ++it, ptr+=len) {
    len = Serialization::decode_vi24(&ptr);
    if(!Condition::eq(
          ptr, len,
          reinterpret_cast<const uint8_t*>(it->c_str()), it->length()))
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



}}}

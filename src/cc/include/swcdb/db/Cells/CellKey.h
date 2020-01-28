/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_CellKey_h
#define swcdb_db_cells_CellKey_h

#include "swcdb/core/Serialization.h"
#include "swcdb/db/Cells/Comparators.h"

namespace SWC { namespace DB { namespace Cell {

class Key {
  public:

  typedef std::shared_ptr<Key> Ptr;

  explicit Key(bool own = true): own(own), count(0), size(0), data(0) { }

  explicit Key(const Key &other)
      : own(other.size), count(other.count), size(other.size), 
        data(own ? (uint8_t*)memcpy(new uint8_t[size], other.data, size) : 0) {
  }

  void copy(const Key &other) {
    free(); 
    own   = true;
    if(size = other.size) {
      data = new uint8_t[size];
      memcpy(data, other.data, size);
      count = other.count;
    }
  }

  virtual ~Key() {
    if(own && data)
      delete [] data;
  }

  void free() {
    if(own && data)
      delete [] data;
    data = 0;
    size = 0;
    count = 0;
  }
  
  const bool sane() const {
    return (count && size && data) || (!count && !size && !data);
  }

  void add(const std::string& fraction) {
    add((const uint8_t*)fraction.data(), fraction.length());
  }

  void add(const char* fraction) {
    add((const uint8_t*)fraction, strlen(fraction));
  }

  void add(const char* fraction, uint32_t len) {
    add((const uint8_t*)fraction, len);
  }

  void add(const uint8_t* fraction, uint32_t len) {
    uint8_t* fraction_ptr = 0;
    add(fraction, len, &fraction_ptr, 0);
  }

  void add(const uint8_t* fraction, uint32_t len, 
           uint8_t** fraction_ptr, uint8_t reserve) {
    uint32_t prev_size = size;
    size += reserve + Serialization::encoded_length_vi32(len) + len;

    uint8_t* data_tmp = new uint8_t[size];
    if(data) {
      memcpy(data_tmp, data, prev_size);
      if(own)
        delete [] data;
    }
    own = true;
    data = data_tmp;
    
    uint8_t* ptr_tmp = data + prev_size;
    *fraction_ptr = ptr_tmp;
    ptr_tmp += reserve;
    Serialization::encode_vi32(&ptr_tmp, len);
    memcpy(ptr_tmp, fraction, len);
    ++count;
  }

  void insert(uint32_t idx, const std::string& fraction) {
    insert(idx, (const uint8_t*)fraction.data(), fraction.length());
  }

  void insert(uint32_t idx, const char* fraction) {
    insert(idx, (const uint8_t*)fraction, strlen(fraction));
  }

  void insert(uint32_t idx, const char* fraction, uint32_t len) {
    insert(idx, (const uint8_t*)fraction, len);
  }

  void insert(uint32_t idx, const uint8_t* fraction, uint32_t len) {
    uint8_t* fraction_ptr = 0;
    insert(idx, fraction, len, &fraction_ptr, 0);
  }

  void insert(uint32_t idx, const uint8_t* fraction, uint32_t len, 
              uint8_t** fraction_ptr, int8_t reserve) {
    if(!data || idx >= count) {
      add(fraction, len, fraction_ptr, reserve);
      return;
    }

    uint32_t prev_size = size;
    uint32_t f_size = reserve + Serialization::encoded_length_vi32(len) + len;
    size += f_size;

    uint8_t* data_tmp = new uint8_t[size];    
    const uint8_t* ptr_tmp = data;

    uint32_t pos = 0;
    uint32_t offset = 0;

    for(;;) {
      if(idx == pos++) {
        if(offset) 
          memcpy(data_tmp, data, offset);
        *fraction_ptr = data_tmp + offset + reserve;
        Serialization::encode_vi32(fraction_ptr, len);
        memcpy(*fraction_ptr, fraction, len);
        *fraction_ptr += len;
        break;
      }
      ptr_tmp += reserve;
      ptr_tmp += Serialization::decode_vi32(&ptr_tmp);
      offset += ptr_tmp-data;
    }
    
    if(prev_size-offset)
      memcpy(*fraction_ptr, ptr_tmp, prev_size-offset);
    
    *fraction_ptr -= f_size;
    if(own)
      delete [] data;
    else
      own = true;
    data = data_tmp;
    ++count;
  }
  
  void remove(uint32_t idx, bool recursive=false, uint8_t reserved=0) {
    if(!data || idx >= count)
      return;

    const uint8_t* ptr_tmp = data;
    if(!own) {
      own = true;
      data = new uint8_t[size];      
      memcpy(data, ptr_tmp, size);
      ptr_tmp = data;
    }

    uint8_t* begin;
    for(uint32_t offset = 0; offset < count; ++offset) {
      begin = (uint8_t*)ptr_tmp;
      ptr_tmp += reserved;
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
        data = new uint8_t[size];
        memcpy(data, ptr_tmp, size);
      } else {
        data = 0;
        size = 0;
      }
      delete ptr_tmp;
      break;
    }
  }

  const std::string get_string(uint32_t idx, uint8_t reserved=0) const {
    const char* fraction;
    uint32_t length = 0;
    get(idx, &fraction, &length, reserved);
    return std::string(fraction, length);
  }

  void get(uint32_t idx, const char** fraction, uint32_t* length, 
           uint8_t reserved=0) const {
    const uint8_t* fraction_ptr = 0;
    get(idx, fraction, length, &fraction_ptr, reserved);
  }

  void get(uint32_t idx, const char** fraction, uint32_t* length, 
           const uint8_t** fraction_ptr, uint8_t reserved) const { 
    if(!data || idx > count) {
      *fraction = 0;
      *length = 0;
      *fraction_ptr = 0;
      return;
    }
    uint32_t len = 0;
    const uint8_t* ptr = 0;
    for(const uint8_t* ptr_tmp = data; ; ptr_tmp += len) {
      ptr = ptr_tmp;
      ptr_tmp += reserved;
      len = Serialization::decode_vi32(&ptr_tmp);
      if(!idx--) { 
        *fraction = (const char*)ptr_tmp;
        *length = len;
        *fraction_ptr = ptr;
        return;
      }
    }
  }

  const bool equal(const Key &other) const {
    return size == other.size && count == other.count 
      && ((!data && !other.data) || memcmp(data, other.data, size) == 0);
  }

  const Condition::Comp compare(const Key &other, uint32_t fractions=0, 
                                bool empty_ok=false) const {
    Condition::Comp comp = Condition::EQ;
    const uint8_t* ptr = data;
    uint32_t len = 0;

    const uint8_t* ptr_other = other.data;
    uint32_t len_other = 0;
    
    uint32_t max = fractions ? fractions 
                  : (count > other.count ? count : other.count);
    for(uint32_t c = 0; c<max; ++c, ptr += len, ptr_other += len_other) {

      if(c == count || c == other.count) {
        comp = count > other.count ? Condition::LT : Condition::GT;
        break;
      } 

      len = Serialization::decode_vi32(&ptr);
      len_other = Serialization::decode_vi32(&ptr_other);
      if((comp = empty_ok && !len
                ? Condition::EQ
                : Condition::condition(ptr, len, ptr_other, len_other))
          != Condition::EQ)
        break;
    }
    return comp;
  }
  
/*
  size_t fractions(uint8_t offset=0) {
    uint32_t tmp_count = 0;
    const uint8_t* ptr = data + offset;
    const uint8_t* ptr_end = data + size;

    for(; ptr<ptr_end; ptr += Serialization::decode_vi32(&ptr) + offset)
      ++tmp_count;
    count = tmp_count;
    return tmp_count;
  }
*/
  const bool empty() const {
    return !count;
  }
  
  const uint32_t encoded_length() const {
    return Serialization::encoded_length_vi32(count) + size;;
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, count);
    if(size) {
      memcpy(*bufp, data, size);
      *bufp += size;
    }
  }

  void decode(const uint8_t **bufp, size_t* remainp, 
              bool owner=false, int8_t reserved=0) {
    free();
    own = owner;
    if(count = Serialization::decode_vi32(bufp, remainp)) {
      const uint8_t* ptr_start = *bufp;
      for(uint32_t n=0; n<count; ++n) {
        *bufp += reserved;
        *bufp += Serialization::decode_vi32(bufp);
      }
      size = *bufp - ptr_start;
      *remainp -= size;
      if(own) {
        data = new uint8_t[size];
        memcpy(data, ptr_start, size);
      } else {
        data = (uint8_t*)ptr_start;
      }
    }
  }

  const std::string to_string() const {
    std::string s("Key(");
    s.append("sz=");
    s.append(std::to_string(count));
    s.append(" len=");
    s.append(std::to_string(size));
    s.append(" fractions=[");
    uint32_t len = 0;
    const uint8_t* ptr = data;
    for(uint32_t n=0; n<count; ++n,ptr+=len) {
      s.append("(");
      len = Serialization::decode_vi32(&ptr);
      s.append(std::string((const char*)ptr, len));
      s.append("),");
    }
    s.append("])");
    return s;
  }
  
  void display(std::ostream& out, bool pretty=false) const {
    out << "["; 
    if(!count) {
      out << "]"; 
      return;
    }
      
    uint32_t len;
    const uint8_t* ptr = data;
    char hex[2];
    for(uint32_t n=0; n<count; ) {
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

  void display_details(std::ostream& out, bool pretty=false) const {
    out << "size=" << size << " count=" << count << " fractions=[";
    uint32_t len;
    const uint8_t* ptr = data;
    char hex[2];
    for(uint32_t n=0; n<count; ) {
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

  bool      own;
  uint32_t  count;
  uint32_t  size;
  uint8_t*  data;
};

}}}

#endif
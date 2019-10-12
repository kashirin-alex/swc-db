/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_CellKey_h
#define swcdb_db_cells_CellKey_h

#include "swcdb/lib/core/Serialization.h"
#include "Comparators.h"


namespace SWC { namespace DB { namespace Cell {

class Key {
  public:

  explicit Key(bool own = true): own(own), count(0), size(0), data(0) {}

  explicit Key(const Key &other) {
    copy(other);
  }

  void copy(const Key &other) {
    //std::cout << " copy(const Key &other) " << other.to_string() << "\n";
    free(); 
    own   = true;
    count = other.count;
    size  = other.size;
    if(size > 0) {
      data = new uint8_t[size];
      memcpy(data, other.data, size);
    } else 
      data = 0;
  }

  virtual ~Key(){
    free();
  }

  inline void free(){
    if(own && data != 0) {
      delete [] data;
      data = 0;
    }
    size = 0;
    count = 0;
  }

  inline void add(const std::string fraction) {
    add((const uint8_t*)fraction.data(), fraction.length());
  }

  inline void add(const char* fraction) {
    add((const uint8_t*)fraction, strlen(fraction));
  }

  inline void add(const char* fraction, uint32_t len) {
    add((const uint8_t*)fraction, len);
  }

  inline void add(const uint8_t* fraction, uint32_t len) {
    uint8_t* fraction_ptr = 0;
    add(fraction, len, &fraction_ptr, 0);
  }

  inline void add(const uint8_t* fraction, uint32_t len, 
                  uint8_t** fraction_ptr, int8_t reserve) {
    uint32_t prev_size = size;
    size += reserve + Serialization::encoded_length_vi32(len) + len;

    uint8_t* data_tmp = new uint8_t[size];
    if(data != 0) {
      memcpy(data_tmp, data, prev_size);
      if(own)
        delete [] data;
      else
        own = true;
    }
    data = data_tmp;
    *fraction_ptr = data + prev_size;
    
    uint8_t* ptr_tmp = *fraction_ptr + reserve;
    Serialization::encode_vi32(&ptr_tmp, len);
    memcpy(ptr_tmp, fraction, len);
    ++count;
  }

  inline void insert(uint32_t idx, const std::string fraction) {
    insert(idx, (const uint8_t*)fraction.data(), fraction.length());
  }

  inline void insert(uint32_t idx, const char* fraction) {
    insert(idx, (const uint8_t*)fraction, strlen(fraction));
  }

  inline void insert(uint32_t idx, const char* fraction, uint32_t len) {
    insert(idx, (const uint8_t*)fraction, len);
  }

  inline void insert(uint32_t idx, const uint8_t* fraction, uint32_t len) {
    uint8_t* fraction_ptr = 0;
    insert(idx, fraction, len, &fraction_ptr, 0);
  }

  inline void insert(uint32_t idx, const uint8_t* fraction, uint32_t len, 
                     uint8_t** fraction_ptr, int8_t reserve) {
    if(data == 0 || idx >= count) {
      add(fraction, len, fraction_ptr, reserve);
      return;
    }

    uint32_t prev_size = size;
    uint32_t f_size = reserve + Serialization::encoded_length_vi32(len) + len;
    size += f_size;

    uint8_t* data_tmp = new uint8_t[size];
    uint8_t* ptr_tmp = data;

    uint32_t pos = 0;
    uint32_t offset = 0;

    for(;;) {
      if(idx == pos++) {
        if(offset != 0)
          memcpy(data_tmp, data, offset);
        *fraction_ptr = data_tmp + offset + reserve;
        Serialization::encode_vi32(fraction_ptr, len);
        memcpy(*fraction_ptr, fraction, len);
        *fraction_ptr += len;
        break;
      }
      ptr_tmp += reserve;
      ptr_tmp += Serialization::decode_vi32((const uint8_t**)&ptr_tmp);
      offset += ptr_tmp-data;
    }
    
    if(prev_size-offset > 0)
      memcpy(*fraction_ptr, ptr_tmp, prev_size-offset);
    
    *fraction_ptr -= f_size;
    if(own)
      delete [] data;
    else
      own = true;
    data = data_tmp;
    ++count;
  }

  inline void get(uint32_t idx, char** ptr, uint32_t* length) {
    uint8_t* fraction_ptr = 0;
    get(idx, ptr, length, &fraction_ptr, 0);
  }

  inline void get(uint32_t idx, char** ptr, uint32_t* length, 
                  uint8_t** fraction_ptr, uint8_t offset) {
    *ptr = 0;
    *length = 0;
    if(idx > count)
      return;
   
    uint8_t* ptr_tmp = data;
    for(;;) {
      *fraction_ptr = ptr_tmp;
      ptr_tmp += offset;
      *length = Serialization::decode_vi32((const uint8_t**)&ptr_tmp);
      if(idx-- == 0)
        break;
      ptr_tmp += *length;
    }
    *ptr = (char*)(ptr_tmp);
    
    /*
    uint8_t* ptr_tmp = data + offset;
    const uint8_t* ptr_end = data + size;

    while(ptr_tmp < ptr_end){
      *fraction_ptr = ptr_tmp;
      *length = Serialization::decode_vi32((const uint8_t**)&ptr_tmp);
      if(idx-- == 0){
        *ptr = (char*)(ptr_tmp);
        return; 
      }
      ptr_tmp += *length + offset;
    }
    *length = 0;
    */
  }

  bool const equal(const Key &other) const {
    return size == other.size && count == other.count 
      && ((data == 0 && other.data == 0) || 
          memcmp(data, other.data, size) == 0);
  }
  
  Condition::Comp compare(const Key &other, uint32_t fractions=0) {

    const uint8_t* ptr_tmp = data;
    const uint8_t* ptr_end = data + size;
    uint32_t idx = 0;
    uint32_t len;
    const uint8_t* ptr = 0;

    const uint8_t* ptr_tmp_other = other.data;
    const uint8_t* ptr_end_other = other.data + other.size;
    uint32_t idx_other = 0;
    uint32_t len_other;
    const uint8_t* ptr_other = 0;
    
    Condition::Comp comp = Condition::EQ;
    do {

      while(ptr_tmp < ptr_end){
        len = Serialization::decode_vi32(&ptr_tmp);
        ptr = ptr_tmp;
        ptr_tmp += len;
        idx++;
        break;
      }

      while(ptr_tmp_other < ptr_end_other){
        len_other = Serialization::decode_vi32(&ptr_tmp_other);
        ptr_other = ptr_tmp_other;
        ptr_tmp_other += len_other;
        idx_other++;
        break;
      }
      
      if(idx == idx_other){

        comp = Condition::condition(ptr, len, ptr_other, len_other);
        if(comp != Condition::EQ) 
          return comp;

        if(fractions && fractions == idx)
          break;

      } else 
        return idx > idx_other? Condition::LT : Condition::GT;
      
    } while(ptr_tmp < ptr_end || ptr_tmp_other < ptr_end_other);

    return comp;
  }

  inline size_t fractions(uint8_t offset=0) {
    uint32_t tmp_count = 0;
    uint8_t* ptr = data + offset;
    const uint8_t* ptr_end = data+size;

    for(;ptr<ptr_end; 
        ptr += Serialization::decode_vi32((const uint8_t**)&ptr) + offset)
      tmp_count++;
    count = tmp_count;
    return tmp_count;
  }

  const bool empty() const {
    return count == 0;
  }
  
  const uint32_t encoded_length() const {
    return Serialization::encoded_length_vi32(count) + size;;
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, count);
    memcpy(*bufp, data, size);
    *bufp += size;
  }

  void decode(const uint8_t **bufp, size_t* remainp, 
              bool owner=false, int8_t reserved=0){
    own = owner;
    count = Serialization::decode_vi32(bufp, remainp);
    data = (uint8_t *)*bufp;
    for(uint32_t n=0; n<count; n++){
      *bufp += reserved;
      *bufp += Serialization::decode_vi32(bufp);
    }
    size = *bufp - data;
    *remainp -= size;
    
    if(size == 0) {
      data = 0;
      count = 0;
      return;
    }
    if(own) {
      uint8_t* ptr = data;
      data = new uint8_t[size];
      memcpy(data, ptr, size);
    }
  }

  const std::string to_string() const {
    std::string s("Key(");
    s.append("sz=");
    s.append(std::to_string(count));
    s.append(" len=");
    s.append(std::to_string(size));
    s.append(" fractions=[");
    uint32_t len;
    uint8_t* ptr = data;
    for(uint32_t n=0; n<count; n++) {
      s.append("(");
      len = Serialization::decode_vi32((const uint8_t**)&ptr);
      char c;
      for(int i=0; i<len;i++) {
        c = *(ptr+i);
        s.append(std::string(&c, 1));
      }
      s.append("),");
      ptr += len;
    }
    s.append("])");
    return s;
  }

  bool      own;
  uint32_t  count;
  uint32_t  size;
  uint8_t*  data;
};

}}}

#endif





/* // Key ( based on null delimited fractions)
class Key {
  public:

  Key(): data(0), size(0) {}

  virtual ~Key(){
    if(data != 0)
      delete [] data;
  }

  inline void add(const std::string fraction) {
    add((const uint8_t*)fraction.data(), fraction.length());
  }

  inline void add(const char* fraction) {
    add((const uint8_t*)fraction, strlen(fraction));
  }

  inline void add(const char* fraction, uint32_t len) {
    add((const uint8_t*)fraction, len);
  }

  inline void add(const uint8_t* fraction, uint32_t len, uint8_t offset=0) {
    len++;
    if(data == 0) {
      data = new uint8_t[len];
      memcpy(data+offset, fraction, len-1);
    } else {
      uint8_t* data_tmp = new uint8_t[size+len+1];
      memcpy(data_tmp, data, size);
      delete [] data;
      data = data_tmp;
      memcpy(data+size+offset, fraction, len-1);
    }
    size += len;
    *(data+size-1) = 0;
  }

  bool equal(const Key &other) {
    return size != other.size || memcmp(data, other.data, size) != 0;
  }

  inline void get(uint32_t fraction, char** ptr, uint32_t* length, uint8_t offset=0) {
    *ptr=0;
    *length=0;
    uint32_t count = 0;
    uint32_t last = offset;
    for(uint32_t n=offset;n<size;n++) {
      if(*(data+n) != 0)
        continue;
      if(fraction == count++){
        *length = n-last;
        *ptr = (char*)(data+last);
        break; 
      }
      n+=offset;
      last = n+1;
    }
  }

  inline size_t fractions(uint8_t offset=0) {
    uint32_t count = 0;
    for(uint32_t n=offset;n<size;n++) {
      if(*(data+n) == 0) {
        count++;
        n+=offset;
      }
    }
    return count;
  }

  const std::string to_string(){
    std::string s("Key(");
    uint32_t last = 0;
    for(uint32_t n=0; n<size;n++) {
      if(*(data+n) != 0)
        continue;
      s.append(std::string((const char*)data+last, n-last));
      s.append(", ");
      last = n;
    }
    s.append(")");
    return s;
  }

  uint8_t* data;
  uint32_t size;
};


namespace Specs {
      
class Key : public DB::Cell::Key {
  public:

  Key() {}

  virtual ~Key(){}

  inline void add(const std::string fraction, Condition::Comp comp) {
    add((const uint8_t*)fraction.data(), fraction.length(), comp);
  }

  inline void add(const char* fraction, Condition::Comp comp) {
    add((const uint8_t*)fraction, strlen(fraction), comp);
  }

  inline void add(const char* fraction, uint32_t len, Condition::Comp comp) {
    add((const uint8_t*)fraction, len, comp);
  }

  inline void add(const uint8_t* fraction, uint32_t len, Condition::Comp comp) {
    DB::Cell::Key::add(fraction, ++len, 1);
    *(data+size-len-1) = (uint8_t)comp;
  }

  bool equal(const Key &other) {
    return size != other.size || memcmp(data, other.data, size) != 0;
  }

  inline void get(uint32_t fraction, char** ptr, uint32_t* length, Condition::Comp* comp) {
    *length = 0;
    *ptr = 0;
    DB::Cell::Key::get(fraction, ptr, length, 1);
    if(*ptr != 0) 
      *comp = (Condition::Comp)*((uint8_t*)*ptr-1);
  }
  
  inline size_t fractions() {
    return DB::Cell::Key::fractions(1);
  }
  

  inline bool is_matching(const DB::Cell::Key &cell_key) {

    uint32_t n = 1;
    uint8_t* ptr = 0;
    uint32_t len = 0;
    uint32_t last = 1;
    uint32_t count = 0;
    uint8_t  comp;

    uint32_t n_other = 0;
    uint8_t* ptr_other = 0;
    uint32_t len_other = 0;
    uint32_t last_other = 0;
    uint32_t count_other = 0;

    do {
      for(;n<size;n++) {
        if(*(data+n) == 0){
          len = n-last;
          ptr = data+last;
          comp = *(ptr-1);
          last = ++n;
          last++;
          ++count;
          break;
        }
      }
      for(;n_other<cell_key.size;n_other++) {
        if(*(cell_key.data+n_other) == 0){
          len_other= n_other-last_other;
          ptr_other = cell_key.data+last_other;
          last_other = ++n_other;
          ++count_other;
          break;
        }
      }
      
      if(count == count_other) {
        if(!Condition::is_matching((Condition::Comp)comp, ptr, len, ptr_other, len_other))
          return false;

      } else {
        switch(comp) {
          case Condition::LT:
            return count > count_other;
          case Condition::LE:
            return count > count_other;
          case Condition::GT:
            return count < count_other;
          case Condition::GE:
            return count < count_other;
          case Condition::PF:
            return count < count_other;
          case Condition::RE:
            return count < count_other;
          case Condition::NE:
            return true;
          case Condition::NONE:
            return true;
          default:
            return false;
        }
      }
      
    } while(n != size || n_other != cell_key.size);

    return true;
  }

  const std::string to_string(){
    std::string s("Key(");
    uint32_t last = 0;
    for(auto n=0; n<size;n++) {
      if(*(data+n) != 0)
        continue;
      s.append(std::string((const char*)data+last, n-last));
      s.append(", ");
      last = n;
    }
    s.append(")");
    return s;
  }

};

} // namespace Specs

*/

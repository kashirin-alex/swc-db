/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h

#include "Cell.h"

namespace SWC { namespace DB { namespace Specs {
      
class Key : public DB::Cell::Key {
  public:

  typedef std::shared_ptr<Key> Ptr;

  explicit Key() {}
  
  explicit Key(const Key &other){
    copy(other);
  }

  explicit Key(const DB::Cell::Key &cell_key, Condition::Comp comp, 
               uint32_t offset=0){
    set(cell_key, comp, offset);
  }

  virtual ~Key(){
    free();
  }

  inline void set(const DB::Cell::Key &cell_key, Condition::Comp comp,
                  uint32_t offset=0) {
    free();
    own   = true;
    count = cell_key.count;
    size  = cell_key.size+count;
    if(size == 0) 
      return;
    
    data = new uint8_t[size];
    uint8_t* data_ptr = data;
    
    uint32_t len;
    const uint8_t* ptr = (const uint8_t*)cell_key.data;
    const uint8_t* ptr_len;
    for(offset; offset<count; offset++) {
      *data_ptr++ = (uint8_t)comp;
      ptr_len = ptr; 
      len = Serialization::decode_vi32(&ptr_len);
      len += ptr_len-ptr;
      memcpy(data_ptr, ptr, len);
      data_ptr += len;
      ptr += len;
    }
  }

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
    uint8_t* fraction_ptr = 0;
    DB::Cell::Key::add(fraction, len, &fraction_ptr, 1);
    *fraction_ptr = (uint8_t)comp;
  }

  inline void set(int32_t idx, Condition::Comp comp) {
    if(count == 0)
      return;

    uint8_t* ptr = data;
    for(int32_t n=0; n<count;
        n++, ptr += Serialization::decode_vi32((const uint8_t**)&++ptr)) {
      if(idx == -1 || n == idx) {
        *ptr = (uint8_t)comp;
        if(idx > -1)
          break;
      }
    }
  }
  
  inline void insert(uint32_t idx, const std::string& fraction,
                     Condition::Comp comp) {
    insert(idx, (const uint8_t*)fraction.data(), fraction.length(), comp);
  }

  inline void insert(uint32_t idx, const char* fraction,
                     Condition::Comp comp) {
    insert(idx, (const uint8_t*)fraction, strlen(fraction), comp);
  }

  inline void insert(uint32_t idx, const char* fraction, uint32_t len,
                     Condition::Comp comp) {
    insert(idx, (const uint8_t*)fraction, len, comp);
  }

  inline void insert(uint32_t idx, const uint8_t* fraction, uint32_t len, 
                    Condition::Comp comp) {
    uint8_t* fraction_ptr = 0;
    DB::Cell::Key::insert(idx, fraction, len, &fraction_ptr, 1);
    *fraction_ptr = (uint8_t)comp;
  }

  inline void get(uint32_t idx, char** ptr, uint32_t* length, Condition::Comp* comp) {
    *length = 0;
    *ptr = 0;
    uint8_t* fraction_ptr = 0;
    DB::Cell::Key::get(idx, ptr, length, &fraction_ptr, 1);
    if(fraction_ptr != 0) 
      *comp = (Condition::Comp)*(fraction_ptr);
  }

  void get(DB::Cell::Key &cell_key) {
    cell_key.free();
    
    if(count > 0) {
      cell_key.own = true;
      cell_key.count = count;
      cell_key.size  = size-count;
      cell_key.data = new uint8_t[cell_key.size];
      const uint8_t* ptr = data;
      const uint8_t* ptr_len;
      uint8_t* data_ptr = cell_key.data;
      uint32_t len;
      for(uint8_t i=0; i<count; i++) {
        ptr_len = ++ptr;
        ptr += Serialization::decode_vi32(&ptr);
        memcpy(data_ptr, ptr_len, len = ptr-ptr_len);
        data_ptr += len;
      }
    }
  }

  inline void remove(uint32_t idx, bool recursive=false) {
    DB::Cell::Key::remove(idx, recursive, 1);
  }

  const bool equal(const Key &other) const {
    return DB::Cell::Key::equal(other);
  }
  
  inline size_t fractions() {
    return DB::Cell::Key::fractions(1);
  }
  
  inline const bool is_matching(const DB::Cell::Key &other) const {
    return is_matching(other.data, other.data + other.size, 0);
  }

  inline const bool is_matching(const Key &other) const {
    return is_matching(other.data, other.data + other.size, 1);
  }

  inline const bool is_matching(const uint8_t* ptr_tmp_other, 
                                const uint8_t* ptr_end_other,
                                int8_t reserved) const {
    const uint8_t* ptr_tmp = data;
    const uint8_t* ptr_end = data + size;

    uint32_t idx = 0;
    uint32_t len;
    const uint8_t* ptr = 0;

    uint32_t idx_other = 0;
    uint32_t len_other;
    const uint8_t* ptr_other = 0;

    Condition::Comp comp = Condition::NONE;
    do {

      while(ptr_tmp < ptr_end){
        comp = (Condition::Comp)*ptr_tmp++;
        len = Serialization::decode_vi32(&ptr_tmp);
        ptr = ptr_tmp;
        ptr_tmp += len;
        idx++;
        break;
      }

      while(ptr_tmp_other < ptr_end_other){
        ptr_tmp_other += reserved;
        len_other = Serialization::decode_vi32(&ptr_tmp_other);
        ptr_other = ptr_tmp_other;
        ptr_tmp_other += len_other;
        idx_other++;
        break;
      }
      
      if(idx == idx_other){
        if(!Condition::is_matching(comp, ptr, len, ptr_other, len_other))
          return false;
        
      } else {
        switch(comp) {
          case Condition::LT:
            return count == 0 || idx > idx_other;
          case Condition::LE:
            return count == 0 || idx > idx_other;
          case Condition::GT:
            return count == 0 || idx < idx_other;
          case Condition::GE:
            return count == 0 || idx < idx_other;
          case Condition::PF:
            return idx < idx_other;
          case Condition::RE:
            return idx < idx_other;
          case Condition::NE:
            return true;
          case Condition::NONE:
            return true;
          default: // Condition::EQ:
            return false;
        }
      }

    } while(ptr_tmp < ptr_end || ptr_tmp_other < ptr_end_other);

    return true;
  }

  void decode(const uint8_t **bufp, size_t *remainp, bool owner=false){
    return DB::Cell::Key::decode(bufp, remainp, owner, 1); 
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
      s.append(Condition::to_string(*ptr++));
      s.append("(");
      len = Serialization::decode_vi32((const uint8_t**)&ptr);
      s.append(std::string((const char*)ptr, len));
      s.append("),");
      ptr += len;
    }
    s.append("])");
    return s;
  }

};


}}}
#endif

/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h

#include "CellKey.h"
#include "Comparators.h"

namespace SWC { namespace DB { namespace Specs {
      
class Key : public DB::Cell::Key {
  public:

  explicit Key() {}
  
  explicit Key(const Key &other){
    copy(other);
  }

  virtual ~Key(){
    free();
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
  
  inline void add(const uint8_t* fraction, uint32_t len, Condition::Comp comp) {
    uint8_t* fraction_ptr = 0;
    DB::Cell::Key::add(fraction, len, &fraction_ptr, 1);
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

  bool equal(const Key &other) {
    return DB::Cell::Key::equal(other);
  }
  
  inline size_t fractions() {
    return DB::Cell::Key::fractions(1);
  }
  
  inline bool is_matching(const DB::Cell::Key &cell_key) {
      
    uint32_t idx = 0;
    uint8_t* ptr = 0;
    uint32_t len;
    uint8_t* ptr_tmp = data;
    const uint8_t* ptr_end = data + size;

    uint32_t idx_other = 0;
    uint8_t* ptr_other = 0;
    uint32_t len_other;
    uint8_t* ptr_tmp_other = cell_key.data;
    const uint8_t* ptr_end_other = cell_key.data + cell_key.size;

    Condition::Comp comp = Condition::NONE;
    do {

      while(ptr_tmp < ptr_end){
        comp = (Condition::Comp)*ptr_tmp++;
        len = Serialization::decode_vi32((const uint8_t**)&ptr_tmp);
        ptr = ptr_tmp;
        ptr_tmp += len;
        idx++;
        break;
      }

      while(ptr_tmp_other < ptr_end_other){
        len_other = Serialization::decode_vi32((const uint8_t**)&ptr_tmp_other);
        ptr_other = ptr_tmp_other;
        ptr_tmp_other += len_other;
        idx_other++;
        break;
      }
      
      if(!is_matching(idx, idx_other, comp, ptr, len, ptr_other, len_other))
        return false;

    } while(ptr_tmp < ptr_end || ptr_tmp_other < ptr_end_other);

    return true;
  }
  
  inline bool is_matching(const Key &other) {
    
    uint32_t idx = 0;
    uint8_t* ptr = 0;
    uint32_t len;
    uint8_t* ptr_tmp = data;
    const uint8_t* ptr_end = data + size;

    uint32_t idx_other = 0;
    uint8_t* ptr_other = 0;
    uint32_t len_other;
    uint8_t* ptr_tmp_other = other.data;
    const uint8_t* ptr_end_other = other.data + other.size;

    Condition::Comp comp = Condition::NONE;
    do {

      while(ptr_tmp < ptr_end){
        comp = (Condition::Comp)*ptr_tmp++;
        len = Serialization::decode_vi32((const uint8_t**)&ptr_tmp);
        ptr = ptr_tmp;
        ptr_tmp += len;
        idx++;
        break;
      }

      while(ptr_tmp_other < ptr_end_other){
        comp = (Condition::Comp)*ptr_tmp_other++;
        len_other = Serialization::decode_vi32((const uint8_t**)&ptr_tmp_other);
        ptr_other = ptr_tmp_other;
        ptr_tmp_other += len_other;
        idx_other++;
        break;
      }
      
      if(!is_matching(idx, idx_other, comp, ptr, len, ptr_other, len_other))
        return false;

    } while(ptr_tmp < ptr_end || ptr_tmp_other < ptr_end_other);

    return true;
  }

  inline bool is_matching(const uint32_t& idx, 
                          const uint32_t& idx_other, 
                          const Condition::Comp& comp,
                          const uint8_t* ptr, 
                          const uint32_t& len, 
                          const uint8_t* ptr_other, 
                          const uint32_t& len_other) {
    if(idx == idx_other)
      return Condition::is_matching(comp, ptr, len, ptr_other, len_other);

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
      default:
        return false;
    }
  }

  size_t encoded_length() const {
    return Serialization::encoded_length_vi32(count) + size;
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, count);
    memcpy(*bufp, data, size);
    *bufp += size;
  }

  void decode(const uint8_t **bufp, size_t *remainp, bool owner=false){
    own = owner;
    count = Serialization::decode_vi32(bufp, remainp);
    data = (uint8_t *)*bufp;
    for(uint32_t n=0; n<count; n++){
      ++*bufp;
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

  const std::string to_string(){
    std::string s("Key(");
    uint32_t len;
    uint8_t* ptr = data;
    for(uint32_t n=0; n<count; n++) {
      s.append("[comp(");
      s.append(Condition::to_string(*ptr++));
      s.append(") fraction(");
      len = Serialization::decode_vi32((const uint8_t**)&ptr);
      s.append(std::string((const char*)ptr, len));
      s.append(")], ");
      ptr += len;
    }
    s.append(")");
    return s;
  }

};


}}}
#endif

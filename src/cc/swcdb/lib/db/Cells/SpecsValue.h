/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsValue_h
#define swcdb_db_cells_SpecsValue_h

#include "swcdb/lib/core/Serializable.h"
#include "Comparators.h"


namespace SWC { namespace DB { namespace Specs {

class Value {
  public:

  explicit Value(bool own=true)
        : own(own), data(0), size(0), comp(Condition::NONE) {}

  explicit Value(const char* data_n, Condition::Comp comp_n,
                 bool owner=false) {
    set((uint8_t*)data_n, strlen(data_n), comp_n, owner);
  }

  explicit Value(const char* data_n, const uint32_t size_n, 
                 Condition::Comp comp_n, bool owner=false) {
    set((uint8_t*)data_n, size_n, comp_n, owner);
  }

  explicit Value(const uint8_t* data_n, const uint32_t size_n, 
                 Condition::Comp comp_n, bool owner=false) {
    set(data_n, size_n, comp_n, owner);
  }

  explicit Value(int64_t count, Condition::Comp comp_n) {
    set(count, comp_n);
  }

  explicit Value(const Value &other){
    copy(other);
  }

  void set(int64_t count, Condition::Comp comp_n) {
    uint32_t len = Serialization::encoded_length_vi64(count);
    uint8_t data_n[len];
    uint8_t* ptr = data_n;
    Serialization::encode_vi64(&ptr, count);
    set(data_n, len, comp_n, true);
  }

  void set(const char* data_n, Condition::Comp comp_n, bool owner=true) {
    set((uint8_t*)data_n, strlen(data_n), comp_n, owner);
  }

  void copy(const Value &other) {
    free(); 
    set(other.data, other.size, other.comp, true);
  }
  
  void set(const uint8_t* data_n, const uint32_t size_n, 
           Condition::Comp comp_n, bool owner=false) {
    free();

    own   = owner;
    comp = comp_n;
    size = size_n;
    if(size > 0) {
      if (own) {
        data = new uint8_t[size];
        memcpy(data, data_n, size);
      } else {
        data = (uint8_t*)data_n;
      }
    }
  }

  virtual ~Value(){
    //std::cout << " ~Value\n";
    if(own && data != 0) 
      delete [] data;
  }

  void free(){
    if(own && data != 0) {
      delete [] data;
      data = 0;
    }
    size = 0;
  }

  const bool empty() const {
    return comp == Condition::NONE;
  }

  const bool equal(const Value &other) const {
    return size == other.size 
      && ((data == 0 && other.data == 0) || 
          memcmp(data, other.data, size) == 0);
  }

  const size_t encoded_length() const {
    return 1+(
      comp==Condition::NONE? 0: Serialization::encoded_length_vi32(size)+size);
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)comp);
    if(comp != Condition::NONE) {
      Serialization::encode_vi32(bufp, size);
      memcpy(*bufp, data, size);
      *bufp+=size;
    }
  }

  void decode(const uint8_t **bufp, size_t *remainp) {
    own = false;
    comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
    if(comp != Condition::NONE){
      size = Serialization::decode_vi32(bufp, remainp);
      data = (uint8_t*)*bufp;
      *remainp -= size;
      *bufp += size;
    }
  }

  const bool is_matching(const uint8_t *other_data, const uint32_t other_size) const {
    return Condition::is_matching(comp, data, size, other_data, other_size);
  }
  
  const bool is_matching(int64_t other) const {
    const uint8_t* counter = data;
    size_t remain = size;
    return Condition::is_matching(
      comp, Serialization::decode_vi64(&counter, &remain), other);
  }

  const std::string to_string() const {
    std::string s("Value(");
    s.append("size=");
    s.append(std::to_string(size));
    s.append(" ");
    s.append(Condition::to_string(comp));
    s.append("(");
    s.append(std::string((const char*)data, size));
    s.append("))");
    return s;
  }

  bool            own;
  uint8_t*        data;
  uint32_t        size;    
  Condition::Comp comp;

};


}}}
#endif
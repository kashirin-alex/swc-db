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

  explicit Value(const char* data, Condition::Comp comp, 
        bool own=false)
        : own(own), data((uint8_t*)data), size(strlen(data)), comp(comp) {}

  explicit Value(const char* data, const uint32_t size, 
                 Condition::Comp comp, bool own=false)
        : own(own), data((uint8_t*)data), size(size), comp(comp) {}

  explicit Value(const uint8_t* data, const uint32_t size, 
                 Condition::Comp comp, bool own=false)
        : own(own), data((uint8_t*)data), size(size), comp(comp) {}

  explicit Value(const Value &other){
    copy(other);
  }

  void copy(const Value &other) {
    free(); 
    own   = true;
    size  = other.size;
    comp = other.comp;
    if(size > 0) {
      data = new uint8_t[size];
      memcpy(data, other.data, size);
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

  bool equal(const Value &other) {
    return size == other.size 
      && ((data == 0 && other.data == 0) || 
          memcmp(data, other.data, size) == 0);
  }

  size_t encoded_length() const {
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

  void decode(const uint8_t **bufp, size_t *remainp){
    own = false;
    comp = (Condition::Comp)Serialization::decode_i8(bufp, remainp);
    if(comp != Condition::NONE){
      size = Serialization::decode_vi32(bufp, remainp);
      data = (uint8_t*)*bufp;
      *remainp -= size;
      *bufp += size;
    }
  }

  bool is_matching(const uint8_t *other_data, const uint32_t other_size){
    return Condition::is_matching(comp, data, size, other_data, other_size);
  }
  
  /*
  bool is_matching_counter(int64_t other){
    if(matcher == nullptr) matcher = matcher->get_matcher(comp);
    return matcher->is_matching(&data, other);
  }
  */

  const std::string to_string(){
    std::string s("Value(comp(");
    s.append(Condition::to_string(comp));
    s.append(") size(");
    s.append(std::to_string(size));
    s.append(") data(");
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
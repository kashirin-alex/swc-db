/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h

#include "swcdb/db/Cells/Cell.h"

namespace SWC { namespace DB { namespace Specs {
      
class Key : public DB::Cell::Key {
  public:

  typedef std::shared_ptr<Key> Ptr;

  explicit Key() {}
  
  explicit Key(const Key &other) {
    copy(other);
  }

  explicit Key(const DB::Cell::Key &cell_key, Condition::Comp comp, 
               uint32_t offset=0) {
    set(cell_key, comp, offset);
  }

  virtual ~Key() {
    free();
  }

  void set(const DB::Cell::Key &cell_key, Condition::Comp comp,
           uint32_t offset=0) {
    assert(cell_key.sane());
    free();
    own   = true;
    count = cell_key.count;
    size  = cell_key.size+count;
    if(!size) 
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
    assert(sane());
  }

  void add(const std::string fraction, Condition::Comp comp) {
    add((const uint8_t*)fraction.data(), fraction.length(), comp);
  }

  void add(const char* fraction, Condition::Comp comp) {
    add((const uint8_t*)fraction, strlen(fraction), comp);
  }

  void add(const char* fraction, uint32_t len, Condition::Comp comp) {
    add((const uint8_t*)fraction, len, comp);
  }
  
  void add(const uint8_t* fraction, uint32_t len, Condition::Comp comp) {
    uint8_t* fraction_ptr = 0;
    DB::Cell::Key::add(fraction, len, &fraction_ptr, 1);
    *fraction_ptr = (uint8_t)comp;
  }

  void set(int32_t idx, Condition::Comp comp) {
    assert(sane());
    if(!count)
      return;

    const uint8_t* ptr = data;
    for(int32_t n=0; n<count;
        n++, ptr += Serialization::decode_vi32(&++ptr)) {
      if(idx == -1 || n == idx) {
        *(data+(ptr-data)) = (uint8_t)comp;
        if(idx > -1)
          break;
      }
    }
  }
  
  void insert(uint32_t idx, const std::string& fraction, 
              Condition::Comp comp) {
    insert(idx, (const uint8_t*)fraction.data(), fraction.length(), comp);
  }

  void insert(uint32_t idx, const char* fraction,
              Condition::Comp comp) {
    insert(idx, (const uint8_t*)fraction, strlen(fraction), comp);
  }

  void insert(uint32_t idx, const char* fraction, uint32_t len,
              Condition::Comp comp) {
    insert(idx, (const uint8_t*)fraction, len, comp);
  }

  void insert(uint32_t idx, const uint8_t* fraction, uint32_t len, 
              Condition::Comp comp) {
    uint8_t* fraction_ptr = 0;
    DB::Cell::Key::insert(idx, fraction, len, &fraction_ptr, 1);
    *fraction_ptr = (uint8_t)comp;
  }

  const std::string get_string(uint32_t idx) const {    
    return DB::Cell::Key::get_string(idx, 1);
  }

  void get(uint32_t idx, const char** fraction, uint32_t* length, 
           Condition::Comp* comp) const {
    *length = 0;
    *fraction = 0;
    const uint8_t* fraction_ptr = 0;
    DB::Cell::Key::get(idx, fraction, length, &fraction_ptr, 1);
    if(fraction_ptr) 
      *comp = (Condition::Comp)*fraction_ptr;
  }

  void get(DB::Cell::Key &cell_key) const {
    assert(sane());
    cell_key.free();
    
    if(count) {
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
    assert(cell_key.sane());
  }

  void remove(uint32_t idx, bool recursive=false) {
    DB::Cell::Key::remove(idx, recursive, 1);
  }

  bool const equal(const Key &other) const {
    assert(sane());
    assert(other.sane());
    return DB::Cell::Key::equal(other);
  }
  /*
  size_t fractions() {
    return DB::Cell::Key::fractions(1);
  }
  */
  
  const bool is_matching(const DB::Cell::Key &other, 
                         Condition::Comp on_side_for_eq=Condition::NONE) const {
    assert(sane());
    assert(other.sane());
    return is_matching(other.data, other.data + other.size, 0, on_side_for_eq);
  }

  const bool is_matching(const Key &other, 
                         Condition::Comp on_side_for_eq=Condition::NONE) const {
    assert(sane());
    assert(other.sane());
    return is_matching(other.data, other.data + other.size, 1, on_side_for_eq);
  }

  const bool is_matching(const uint8_t* ptr_tmp_other, 
                         const uint8_t* ptr_end_other, int8_t reserved, 
                         Condition::Comp on_side_for_eq=Condition::NONE) const {
    const uint8_t* ptr_tmp = data;
    const uint8_t* ptr_end = data + size;

    uint32_t idx = 0;
    uint32_t len = 0;
    const uint8_t* ptr = 0;

    uint32_t idx_other = 0;
    uint32_t len_other = 0;
    const uint8_t* ptr_other = 0;

    Condition::Comp comp = Condition::NONE;
    do {

      if(ptr_tmp < ptr_end) {
        comp = (Condition::Comp)*ptr_tmp++;
        if(on_side_for_eq != Condition::NONE) {
          if(comp == Condition::RE || comp == Condition::NE)
            comp = Condition::NONE;
          else if(comp == Condition::EQ)
            comp = on_side_for_eq;
          else if(comp == Condition::PF)
            comp = Condition::GE;
        }
        len = Serialization::decode_vi32(&ptr_tmp);
        ptr = ptr_tmp;
        ptr_tmp += len;
        idx++;
      }

      if(ptr_tmp_other < ptr_end_other) {
        ptr_tmp_other += reserved;
        len_other = Serialization::decode_vi32(&ptr_tmp_other);
        ptr_other = ptr_tmp_other;
        ptr_tmp_other += len_other;
        idx_other++;
      }
      
      if(idx == idx_other) {
        if(!Condition::is_matching(comp, ptr, len, ptr_other, len_other))
          return false;
        
      } else {
        switch(comp) {
          case Condition::LT:
          case Condition::LE:
            return !count || idx > idx_other;
          case Condition::GT:
          case Condition::GE:
            return !count || idx < idx_other;
          case Condition::PF:
          case Condition::RE:
            return idx < idx_other;
          case Condition::NE:
          case Condition::NONE:
            return true;
          default: // Condition::EQ:
            return false;
        }
      }

    } while(ptr_tmp < ptr_end || ptr_tmp_other < ptr_end_other);

    return true;
  }

  void decode(const uint8_t **bufp, size_t *remainp, bool owner=false) {
    return DB::Cell::Key::decode(bufp, remainp, owner, 1); 
  }

  const std::string to_string() const {
    assert(sane());
    std::string s("Key(");
    s.append("sz=");
    s.append(std::to_string(count));
    s.append(" len=");
    s.append(std::to_string(size));
    s.append(" fractions=[");
    uint32_t len;
    const uint8_t* ptr = data;
    for(uint32_t n=0; n<count; n++) {
      s.append(Condition::to_string(*ptr++));
      s.append("(");
      len = Serialization::decode_vi32(&ptr);
      s.append(std::string((const char*)ptr, len));
      s.append("),");
      ptr += len;
    }
    s.append("])");
    return s;
  }

  void display(std::ostream& out) const {
    out << "size=" << size << " count=" << count << " fractions=[";
    uint32_t len;
    const uint8_t* ptr = data;
    for(uint32_t n=0; n<count;) {
      out << Condition::to_string(*ptr++);
      len = Serialization::decode_vi32(&ptr);
      out << '"' << std::string((const char*)ptr, len) << '"';
      ptr += len;
      if(++n < count)
        out << ", "; 
    }
    out << "]"; 
  }

};


}}}
#endif

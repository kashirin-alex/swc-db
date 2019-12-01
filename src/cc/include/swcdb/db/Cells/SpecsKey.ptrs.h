/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h

#include "swcdb/db/Cells/Cell.h"

namespace SWC { namespace DB { namespace Specs {
      
class Key {
  
  public:
  
  struct Fraction {
    Condition::Comp comp;
    uint32_t        size;
    uint8_t*        data;
  };

  explicit Key(bool own = true): own(own), count(0), fractions(0) {}

  explicit Key(const Key &other): fractions(0) {
    copy(other);
  }

  void copy(const Key &other) {
    //std::cout << " copy(const Key &other)\n";
    free(); 
    own   = true;
    count = other.count;
    
    if(count > 0) {
      fractions = new Fraction*[count];
      Fraction* f2;
      for(int n =0; n < count; n++){
        f2 = *(other.fractions+n);
        set_fraction(fractions+n, f2->data, f2->size, f2->comp);
      }
    } else 
      fractions = 0;
  }

  virtual ~Key(){
    free();
  }

  inline void free() {
    if(fractions != 0) {
      for(int n =0; n < count;n++){
        if(own)
          delete (*(fractions+n))->data;
        delete *(fractions+n);
      }
      delete [] fractions;
      count = 0;
      fractions = 0;
    }
  }

  static inline 
  void set_fraction(Fraction** f, const uint8_t* data, uint32_t len,
                    Condition::Comp comp) {
    *f = new Fraction({.comp=comp, .size=len, .data=new uint8_t[len]});
    if(data)
      memcpy((*f)->data, data, len);
  }

  inline void set(const DB::Cell::Key &cell_key, Condition::Comp comp) {
    free();

    own   = true;
    count = cell_key.count;
    if(count == 0) 
      return;

    fractions = new Fraction*[count];
    DB::Cell::Key::Fraction* f2;
    for(int n =0; n < count;n++){
      f2 = *(cell_key.fractions+n);
      set_fraction(fractions+n, f2->data, f2->size, comp);
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
    Fraction** tmp = new Fraction*[count+1];
    memcpy(tmp, fractions, count*sizeof(Fraction*));
    delete [] fractions;

    fractions = tmp;
    set_fraction(fractions+count, fraction, len, comp);
    ++count;
    own = true;
  }

  inline void set(int32_t idx, Condition::Comp comp) {
    if(count == 0)
      return;

    if(idx >= 0)
      (*(fractions+idx))->comp = comp;
    else { 
      for(int n =0; n < count;n++)
        (*(fractions+n))->comp = comp;
    }
  }
  
  inline void insert(uint32_t idx, const std::string fraction,
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
    if(fractions == 0 || idx >= count) {
      add(fraction, len, comp);
      return;
    }

    Fraction** tmp = new Fraction*[count+1];
    if(idx)
      memcpy(tmp, fractions+idx-1, idx*sizeof(Fraction*));
    set_fraction(tmp+idx, fraction, len, comp);
    memcpy(tmp+idx+1, fractions+idx, (count-idx)*sizeof(Fraction*));
    delete [] fractions;
    
    fractions = tmp;
    ++count;
    own = true;
  }

  inline void get(uint32_t idx, char** ptr, uint32_t* length, Condition::Comp* comp) {
    Fraction* f = *(fractions+idx);
    *ptr = (char*)f->data;
    *length = f->size;
    *comp = f->comp;
  }

  bool equal(const Key &other) {
    if(count != other.count)
      return false;
    Fraction* f;
    Fraction* f2;
    for(int n =0; n < count;n++){
      f = *(fractions+n);
      f2 = *(other.fractions+n);
      if(f->comp != f2->comp || f->size != f2->size 
        || memcmp(f->data, f2->data, f->size) != 0)
        return false;
    }
    return true;
  }

  inline bool is_matching(const Key &other) {
    if(!count)
      return true;
      
    Fraction* f;
    uint32_t  c1 = 0;
    Fraction* f2;
    uint32_t  c2 = 0;

    do {
      if(c1 < count)
        f = *(fractions+(c1++));
      if(c2 < other.count)
        f2 = *(other.fractions+(c2++));

      if(c1 == c2){
        if(!Condition::is_matching(f->comp, f->data, f->size, f2->data, f2->size))
          return false;
      } else 
        return is_matching(f->comp, c1, c2);

    } while(c1 < count || c2 < other.count);

    return true;
  }

  inline bool is_matching(const DB::Cell::Key &other) {
    if(!count)
      return true;
      
    Fraction* f;
    uint32_t  c1 = 0;
    DB::Cell::Key::Fraction* f2;
    uint32_t  c2 = 0;

    do {
      if(c1 < count)
        f = *(fractions+(c1++));
      if(c2 < other.count)
        f2 = *(other.fractions+(c2++));

      if(c1 == c2){
        if(!Condition::is_matching(f->comp, f->data, f->size, f2->data, f2->size))
          return false;
      } else 
        return is_matching(f->comp, c1, c2);

    } while(c1 < count || c2 < other.count);

    return true;
  }

  inline bool is_matching(Condition::Comp comp, uint32_t c1, uint32_t c2) {
    switch(comp) {
      case Condition::LT:
        return count == 0 || c1 > c2;
      case Condition::LE:
        return count == 0 || c1 > c2;
      case Condition::GT:
        return count == 0 || c1 < c2;
      case Condition::GE:
        return count == 0 || c1 < c2;
      case Condition::PF:
        return c1 < c2;
      case Condition::RE:
        return c1 < c2;
      case Condition::NE:
        return true;
      case Condition::NONE:
        return true;
      default: // Condition::EQ:
        return false;
    }
  }

  bool empty() {
    return count == 0;
  }
  
  uint32_t encoded_length() const {
    uint32_t len = 1+Serialization::encoded_length_vi32(count);
    Fraction* f;
    for(int n=0; n < count;n++){
      f = *(fractions+n);
      len += Serialization::encoded_length_vi32(f->size)+f->size;
    }
    return len;
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, count);

    Fraction* f;
    for(int n=0; n < count;n++){
      f = *(fractions+n);
      *(*bufp++) = (uint8_t)f->comp;
      Serialization::encode_vi32(bufp, f->size);
      memcpy(*bufp, f->data, f->size);
      *bufp += f->size;
    }
  }

  void decode(const uint8_t **bufp, size_t* remainp, 
              bool owner=false){
    own = owner;

    count = Serialization::decode_vi32(bufp, remainp);
    fractions = new Fraction*[count];

    uint32_t len;
    Condition::Comp comp;
    for(int n=0; n < count;n++){
      comp = (Condition::Comp)*(*bufp++);
      len = Serialization::decode_vi32(bufp);
      set_fraction(fractions+n, own?*bufp:0, len, comp);
      if(!own)
        (*(fractions+n))->data = (uint8_t*)*bufp;
      *bufp += len;
    }
  }

  const std::string to_string() const {
    std::string s("Key(");
    s.append("sz=");
    s.append(std::to_string(count));

    s.append(" fractions=[");
    Fraction* f;
    for(uint32_t n=0; n<count; n++) {
      f = *(fractions+n);
      s.append("(");
      char c;
      for(int i=0; i<f->size;i++) {
        c = *(f->data+i);
        s.append(std::string(&c, 1));
      }
      s.append(")");
      s.append(Condition::to_string(f->comp));
      s.append(",");
    }
    s.append("])");
    return s;
  }

  bool        own;

  uint32_t    count;
  Fraction**  fractions;

};


}}}
#endif

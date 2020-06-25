/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_cells_CellKeyArena_h
#define swcdb_db_cells_CellKeyArena_h

#include <vector>
#include "swcdb/core/PageArena.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Comparators.h"


namespace SWC { namespace DB { namespace Cell {
  
using Fraction = Mem::Item::Ptr;

class KeyArena final : public std::vector<Fraction> {
  public:

  typedef std::shared_ptr<KeyArena>  Ptr;
  using std::vector<Fraction>::vector;
  using std::vector<Fraction>::insert;


  explicit KeyArena(const KeyArena &other) {
    assign(other.begin(), other.end());
    for(auto it = begin(); it < end(); ++it)
      (*it)->use();
  }
  
  KeyArena(const KeyArena&&) = delete;

  KeyArena& operator=(const KeyArena&) = delete;

  ~KeyArena() {
    free();
  }

  void free() {
    for(auto it = begin(); it < end(); ++it)
      (*it)->release();
    clear();
  }

  void copy(const KeyArena &other) {
    free();
    assign(other.begin(), other.end());
    for(auto it = begin(); it < end(); ++it)
      (*it)->use();
  }

  bool equal(const KeyArena &other) const {
    return *this == other;
  }

  //add fraction
  void add(const uint8_t* buf, const uint32_t len) {
    push_back(Mem::Item::make(buf, len));
  }

  void add(Fraction fraction) {    
    push_back(fraction->use());
  }

  void add(const std::string& fraction) {
    add((const uint8_t*)fraction.data(), fraction.length());
  }

  void add(const std::string_view& fraction) {
    add((const uint8_t*)fraction.data(), fraction.length());
  }

  void add(const char* fraction) {
    add((const uint8_t*)fraction, strlen(fraction));
  }

  void add(const char* fraction, const uint32_t len) {
    add((const uint8_t*)fraction, len);
  }

  //insert fraction
  void insert(const uint32_t idx, const uint8_t* buf, const uint32_t len) {
    insert(begin()+idx, Mem::Item::make(buf, len));
  }

  void insert(const uint32_t idx, const char* fraction, const uint32_t len) {
    insert(idx, (const uint8_t*)fraction, len);
  }

  void insert(const uint32_t idx, const std::string& fraction) {
    insert(idx, (const uint8_t*)fraction.data(), fraction.length());
  }

  void insert(const uint32_t idx, const std::string_view& fraction) {
    insert(idx, (const uint8_t*)fraction.data(), fraction.length());
  }

  void insert(const uint32_t idx, const char* fraction) {
    insert(idx, (const uint8_t*)fraction, strlen(fraction));
  }

  //set fraction
  void _set(const uint32_t idx, Fraction fraction) {    
    auto it = begin() + idx;
    (*it)->release();
    *it = fraction;
  }

  void set(const uint32_t idx, const uint8_t* buf, const uint32_t len) {
    _set(idx, Mem::Item::make(buf, len));
  }

  void set(const uint32_t idx, Fraction fraction) {    
    _set(idx, fraction->use());
  }

  void set(const uint32_t idx, const char* fraction, const uint32_t len) {
    set(idx, (const uint8_t*)fraction, len);
  }

  void set(const uint32_t idx, const std::string& fraction) {
    set(idx, (const uint8_t*)fraction.data(), fraction.length());
  }

  void set(const uint32_t idx, const std::string_view& fraction) {
    set(idx, (const uint8_t*)fraction.data(), fraction.length());
  }

  //remove fraction
  void remove(const uint32_t idx, bool recursive=false) {
    if(idx >= size())
      return;
    auto it = begin()+idx;
    if(!recursive) {
      (*it)->release();
      erase(it);
    } else {
      for(;it<end();++it) {
        (*it)->release();
        erase(it);
      }
    }
  }

  //get fraction
  std::string_view get(const uint32_t idx) const {
    return (*(begin() + idx))->to_string();
  }

  void get(const uint32_t idx, std::string& fraction) const {
    fraction.append(get(idx));
  }
  
  void get(const uint32_t idx, std::string_view& fraction) const {
    fraction = get(idx);
  }
  
  void get(const uint32_t idx, const char** fraction, uint32_t* len) const {
    auto f = *(begin() + idx);
    *fraction = (const char*)f->data();
    *len = f->size();
  }
  
  //compare fraction
  Condition::Comp compare(const KeyArena& other, uint32_t max=0, 
                          bool empty_ok=false) const {
    Condition::Comp comp = Condition::EQ;
    bool on_fraction = max;
    auto f2 = other.begin();
    for(auto f1 = begin(); f1 < end() && f2 < other.end(); ++f1, ++f2) {
      if(!(*f1)->size() && empty_ok)
        continue;

      if((comp = Condition::condition((*f1)->data(), (*f1)->size(), 
                                      (*f2)->data(), (*f2)->size()))
                  != Condition::EQ || (on_fraction && !--max))
        return comp;
    }
    return size() == other.size() 
          ? comp 
          : (size() > other.size() ? Condition::LT : Condition::GT);
  }

  bool compare(const KeyArena& other, Condition::Comp break_if,
               uint32_t max=0, bool empty_ok=false) const {
    bool on_fraction = max;
    auto f1 = begin();
    auto f2 = other.begin();
    for(; f1 < end() && f2 < other.end(); ++f1, ++f2) {
      if(!(*f1)->size() && empty_ok)
        continue;

      if(Condition::condition((*f1)->data(), (*f1)->size(), 
                              (*f2)->data(), (*f2)->size()) == break_if)
        return false;
      if(on_fraction && !--max)
        return true;
    }
    return size() == other.size() 
          ? true 
          : (size() > other.size() 
            ? break_if != Condition::LT 
            : break_if != Condition::GT);
  }

  //align other KeyArena
  bool align(const KeyArena& other, Condition::Comp comp) {
    bool chg;
    if(chg = empty()) {
      if(chg = !other.empty())
        copy(other);
      return chg;
    }
    auto f2 = other.begin();
    for(auto f1 = begin(); f1 < end() && f2 < other.end(); ++f1, ++f2) {
      if(Condition::condition((const uint8_t*)(*f1)->data(), (*f1)->size(),
                              (const uint8_t*)(*f2)->data(), (*f2)->size()
                              ) == comp) {
        (*f1)->release();
        *f1 = (*f2)->use();
        chg = true;
      }
    }
    for(;f2 < other.end(); ++f2) {
      add(*f2);
      chg = true;
    }
    return chg;
  }

  bool align(KeyArena& start, KeyArena& finish) const {
    bool chg = false;
    uint32_t c = 0;
    for(auto it = begin(); it < end(); ++it, ++c) {

      if(c == start.size()) {
        start.add(*it);
        chg = true;
      } else if(Condition::condition(
                  start[c]->data(), start[c]->size(),
                  (*it)->data(), (*it)->size()
              ) == Condition::LT) {
        start.set(c, *it);
        chg = true;
      }

      if(c == finish.size()) {
        finish.add(*it);
        chg = true;
      } else if(Condition::condition(
                  finish[c]->data(), finish[c]->size(),
                  (*it)->data(), (*it)->size()
                ) == Condition::GT) {
        finish.set(c, *it);
        chg = true;
      }
    }
    return chg;
  }

  //Serialization
  uint32_t encoded_length() const {
    uint32_t len = Serialization::encoded_length_vi32(size());
    for(auto it = begin(); it < end(); ++it)
      len += Serialization::encoded_length_vi32((*it)->size()) + (*it)->size();
    return len;
  }
  
  void encode(uint8_t** bufp) const {
    Serialization::encode_vi32(bufp, size());
    uint32_t len;
    for(auto it = begin(); it < end(); ++it) {
      Serialization::encode_vi32(bufp, len = (*it)->size());
      memcpy(*bufp, (*it)->data(), len);
      *bufp += len;
    }
  }

  void decode(const uint8_t** bufp, size_t* remainp, bool owner = true) {
    free();
    uint32_t len;
    resize(Serialization::decode_vi32(bufp, remainp));
    for(auto it = begin(); it<end(); ++it) {
      len = Serialization::decode_vi32(bufp, remainp);
      *it = Mem::Item::make(*bufp, len);
      *bufp += len;
      *remainp -= len;
    }
  }

  //from/to std::vector<std::string>
  void convert_to(std::vector<std::string>& key) const {
    key.clear();
    key.resize(size());
    auto f = key.begin();
    for(auto it = begin(); it<end(); ++it, ++f)
      f->append((const char*)(*it)->data(), (*it)->size());
  }

  void read(const std::vector<std::string>& key)  {
    free();
    resize(key.size());
    auto it = begin();
    for(auto it_k = key.begin(); it_k<key.end(); ++it_k, ++it)
      *it = Mem::Item::make((const uint8_t *)it_k->data(), it_k->length());
  }

  bool equal(const std::vector<std::string>& key) const {
    if(key.size() != size())
      return false;
    auto f = key.begin(); 
    for(auto it = begin(); it<end(); ++it, ++f) {
      if(!Condition::eq((*it)->data(), (*it)->size(), 
                        (const uint8_t *)f->data(), f->length()))
        return false;
    }
    return true;
  }

  //Output
  std::string to_string() const {
    std::string s("KeyArena(");
    std::streamstring ss;
    display_details(ss, true);
    s.append(ss.str());
    s.append(")");
    return s;
  }

  void display_details(std::ostream& out, bool pretty=true) const {
    out << "size=" << size() << " fractions=";
    display(out, pretty);
  }

  void display(std::ostream& out, bool pretty=true) const {
    out << "["; 
    if(empty()) {
      out << "]"; 
      return;
    }
      
    uint32_t len;
    const uint8_t* ptr;
    char hex[5];
    hex[4] = 0;
    for(auto it = begin(); it < end();) {
      out << '"'; 
      ptr = (*it)->data();
      for(len = (*it)->size(); len; --len, ++ptr) {
        if(*ptr == '"')
          out << '\\';
        if(pretty && (*ptr < 32 || *ptr > 126)) {
          sprintf(hex, "0x%X", *ptr);
          out << hex;
        } else {
          out << *ptr; 
        }
      }
      out << '"'; 
      if(++it < end())
        out << ", "; 
    }
    out << "]"; 
    
  }

};

}}}

#endif
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_Intervals_h
#define swcdb_db_Cells_Intervals_h

#include "swcdb/lib/db/ScanSpecs/ScanSpecs.h"


namespace SWC { 
  

namespace Serialization {
  
inline size_t encoded_length(ScanSpecs::ListKeys &keys){
  size_t len = 0;
  for(auto& k : keys)
    len += k.len+1;
  return Serialization::encoded_length_vi32(len)+len;
}

inline void encode(ScanSpecs::ListKeys &keys, uint8_t **ptr){
  size_t len = 0;
  for(auto& k : keys)
    len += k.len+1;
  Serialization::encode_vi32(ptr, len);

  for(auto& k : keys){
    memcpy(*ptr, k.key, k.len);
    *ptr += k.len;
    **ptr = 0;
    *ptr += 1;
  }
}

inline void decode(ScanSpecs::ListKeys &keys, uint8_t* &bufp, 
                   const uint8_t **ptr, uint32_t len, size_t *remain){
  bufp = new uint8_t[len];
  memcpy(bufp, *ptr, len);
  *remain -= len;
  *ptr += len;

  uint8_t* tmpptr = bufp;
  const uint8_t* ptr_end = tmpptr+len;
  uint32_t num_keys = 0;
  for(uint8_t* i=tmpptr;i<ptr_end;i++) {
    if(*i == 0)
      num_keys++;
  }
  keys.reserve(num_keys);

  while (tmpptr < ptr_end){
    ScanSpecs::Key k;
    k.key = (const char *)tmpptr;
    while (*tmpptr != 0)
      *tmpptr++;
    k.len = tmpptr - (uint8_t *)k.key;
    *tmpptr++;
    keys.push_back(k);
  }
}

} // Serialization  namespacde


  
namespace Cells {

class Intervals {

  /* encoded-format: 
      vi32(keys_begin_len-encoded), keys_begin([chars\0])
      vi32(keys_end_len-encoded), keys_end([chars\0])
      vi64(ts_earliest)
      vi64(ts_latest)
  */

  public:

  typedef std::shared_ptr<Intervals> Ptr;

  Intervals() { }

  virtual ~Intervals(){ 
    clear_keys_begin();
    clear_keys_end();
  }

  const size_t encoded_length(){
    std::lock_guard<std::mutex> lock(m_mutex);

    return Serialization::encoded_length(keys_begin) 
            + Serialization::encoded_length(keys_end) 
            + Serialization::encoded_length_vi64(m_ts_earliest) 
            + Serialization::encoded_length_vi64(m_ts_latest);
  }

  void encode(uint8_t **ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Serialization::encode(keys_begin, ptr);
    Serialization::encode(keys_end, ptr);

    Serialization::encode_vi64(ptr, m_ts_earliest);
    Serialization::encode_vi64(ptr, m_ts_latest);
  }

  void decode(const uint8_t **ptr, size_t *remain){
    decode_begin(ptr, remain);
    decode_end(ptr, remain);
    m_ts_earliest = Serialization::decode_vi64(ptr, remain);
    m_ts_latest = Serialization::decode_vi64(ptr, remain);
  }

  void decode_begin(const uint8_t **ptr, size_t *remain, uint32_t len=0){
    std::lock_guard<std::mutex> lock(m_mutex);

    clear_keys_begin();

    if(len == 0)
      m_serial_begin_len = Serialization::decode_vi32(ptr, remain);
    Serialization::decode(
      keys_begin, m_serial_begin, ptr, m_serial_begin_len, remain);

    for(auto& k : keys_begin)
      k.comp = Comparator::GE;
  }

  void decode_end(const uint8_t **ptr, size_t *remain, uint32_t len=0){
    std::lock_guard<std::mutex> lock(m_mutex);

    clear_keys_end();

    if(len == 0)
      m_serial_end_len = Serialization::decode_vi32(ptr, remain);
    Serialization::decode(
      keys_end, m_serial_end, ptr, m_serial_end_len, remain);

    for(auto& k : keys_end)
      k.comp = Comparator::LE;
  }

  void set_keys_begin(Ptr other){
    size_t len = other->get_keys_begin_len();
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_serial_begin_len = len;
    }
    const uint8_t * ptr = other->get_keys_begin_ptr();
    decode_begin(&ptr, &len, m_serial_begin_len);
  }

  void set_keys_end(Ptr other){
    size_t len = other->get_keys_end_len();
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_serial_end_len = len;
    }
    const uint8_t * ptr = other->get_keys_end_ptr();
    decode_end(&ptr, &len, m_serial_end_len);
  }

  bool equal(const Ptr &other){
    std::lock_guard<std::mutex> lock(m_mutex);
    return(
      other != nullptr &&
      other->get_keys_begin_len() == m_serial_begin_len && 
      other->get_keys_end_len() == m_serial_end_len && 
      Condition::is_equal(
        m_serial_begin, m_serial_begin_len,
        other->get_keys_begin_ptr(), other->get_keys_begin_len()) && 
      Condition::is_equal(
        m_serial_end, m_serial_end_len,
        other->get_keys_end_ptr(), other->get_keys_end_len()));
  }

  const uint8_t* get_keys_begin_ptr() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_serial_begin;
  }
  const uint32_t get_keys_begin_len() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_serial_begin_len;
  }
  const uint8_t* get_keys_end_ptr() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_serial_end;
  }
  const uint32_t get_keys_end_len() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_serial_end_len;
  }

  const int64_t get_ts_latest(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ts_latest;
  }
  const int64_t get_ts_earliest(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ts_earliest;
  }

  void set_ts_latest(int64_t ts){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ts_latest = ts;
  }
  void set_ts_earliest(int64_t ts){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ts_earliest = ts;
  }

  ScanSpecs::ListKeys& get_keys_begin(){
    return keys_begin;
  }
  ScanSpecs::ListKeys& get_keys_end(){
    return keys_end;
  }

  bool is_in(ScanSpecs::ListKeys &keys){
    return is_in_begin(keys) && is_in_end(keys);
  }

  bool is_any_keys_begin() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return keys_begin.size() == 0;
  }
  bool is_any_keys_end() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return keys_end.size() == 0;
  }

  bool is_in_begin(ScanSpecs::ListKeys &keys) {
    if(is_any_keys_begin())
      return true;
      
    std::lock_guard<std::mutex> lock(m_mutex);
    if(keys_begin.size() > keys.size())
      return false;

    for(auto i=0; i<keys.size(); i++){
      if(i > keys_begin.size())
        return true;
      if(!keys_begin[i].is_matching(&keys[i].key, keys[i].len))
        return false;
    }
    return true;
  }
  bool is_in_end(ScanSpecs::ListKeys &keys) {
    if(is_any_keys_end())
      return true;

    std::lock_guard<std::mutex> lock(m_mutex);
    if(keys_end.size() < keys.size())
      return false;

    for(auto i=0; i<keys.size(); i++){
      if(!keys_end[i].is_matching(&keys[i].key, keys[i].len))
        return false;
    }
    return true;
  }


  void expande(const Ptr& other, bool init){
    
    if(!init || !is_in_begin(other->get_keys_begin()))
      set_keys_begin(other);

    if(!init || !is_in_end(other->get_keys_end()))
      set_keys_end(other);

    {
      auto ts = other->get_ts_earliest();
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_ts_earliest == ScanSpecs::TIMESTAMP_NULL || m_ts_earliest > ts)
        m_ts_earliest = ts;

      ts = other->get_ts_latest();
      if(m_ts_latest == ScanSpecs::TIMESTAMP_NULL || m_ts_latest < ts)
        m_ts_latest = ts;
    }
  }

  bool consist(const Ptr& other) {
    return is_in_begin(other->get_keys_end()) && 
           is_in_end(other->get_keys_begin());
  }

  bool consist(ScanSpecs::CellsInterval& intervals) {
    return (intervals.keys_finish.keys.empty() 
            || 
            is_in_begin(intervals.keys_finish.keys))
            && 
           (intervals.keys_start.keys.empty()
            ||
            is_in_end(intervals.keys_start.keys));
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::stringstream ss;
    ss << "Intervals(begin={" << keys_begin << "}, end={" << keys_end << "}"
       << ", earliest=" << m_ts_earliest << ", latest=" << m_ts_latest << ")";
    return ss.str();
  }

  private:

  void clear_keys_begin(){
    if(m_serial_begin != 0 ){
      keys_begin.clear();
      delete [] m_serial_begin;
    }
  }
  
  void clear_keys_end(){
    if(m_serial_end != 0) {
      keys_end.clear();
      delete [] m_serial_end;
    }
  }


  std::mutex     m_mutex;

  uint8_t*       m_serial_begin     = 0;
  uint32_t       m_serial_begin_len = 0;
  uint8_t*       m_serial_end       = 0;
  uint32_t       m_serial_end_len   = 0;
  
  ScanSpecs::ListKeys    keys_begin;
  ScanSpecs::ListKeys    keys_end;

  int64_t m_ts_earliest = ScanSpecs::TIMESTAMP_NULL;
  int64_t m_ts_latest   = ScanSpecs::TIMESTAMP_NULL;

};

} // Cells namespace
}


// R&D class
/*
template<class baseT>
class ArrChain {
  public:

  template <class T = baseT>
  class ItemBase {};

  template <class T = baseT>
  class Item : public ItemBase<T> {
    public:
    typedef Item<T>* Ptr;

    Item(){}

    Item(T item): m_item(item) {}

    virtual ~Item(){
      remove();
    }

    bool set_by_lt(T& item, Ptr& current, bool& chg){
      std::lock_guard<std::mutex> lock(m_mutex);
      
      if(m_item != nullptr) {
        chg = item == m_item;   // identity -eq
        if(chg) {
          chg = item != m_item; // property -ne
          if(chg) {
            ((Ptr)m_prev)->next((Ptr)m_next);
            ((Ptr)m_next)->prev((Ptr)m_prev);
            return false;
          }
          return true;
        }
        if(item < m_item){
          Ptr new_item = new Item<baseT>(item);
          new_item->next(this);
          new_item->prev(m_prev);
          m_prev = new_item;
          return true;
        }
      }

      if(m_next == nullptr){
        Ptr new_item = new Item<baseT>(item);
        if(m_item != nullptr)
          new_item->prev(this); 
        m_next = new_item;
        return true;
      }

      current = m_next;
      return false;
    }

    void consist(T& item, bool &next, bool &found, Ptr& current){
      std::lock_guard<std::mutex> lock(m_mutex);

      if(m_item->consist(item)) // property -lt
        return;

      if(found)
        next = true;
      else {
        item = m_item;
        found = true;
      }
      current = m_next;
      return;
    }


    void next(Ptr next) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_next = next;
    }

    void prev(Ptr prev) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_prev = prev;
    }

    void remove(){
      std::lock_guard<std::mutex> lock(m_mutex);
      ((Ptr)m_prev)->next((Ptr)m_next);
      ((Ptr)m_next)->prev((Ptr)m_prev);
    }

    private:
    std::mutex  m_mutex;
    T           m_item;
    Ptr         m_next=nullptr;
    Ptr         m_prev=nullptr;
  };

  
  ArrChain(): first(new Item<baseT>()) {}
  virtual ~ArrChain(){}

  bool consist(baseT& item, bool &next){
    next = false;
    Item<baseT>* current = first;
    bool found = false;
    do{
      current->consist(item, next, found, current);
    } while(current != nullptr && (!found || (found && next)));
    return found;
  }

  void set_by_lt(baseT& item){
    bool chg;
    Item<baseT>* current = first;
    while(!current->set_lowerthan(item, current, chg) && current != nullptr){
      if(chg)
        current = first;
    }
  }

  void remove(baseT& item){
    first->remove(item);
  }

  const Item<baseT>* first;
};

inline static  ArrChain<Cells::Intervals::Ptr> ranggsfgfghes;
*/

#endif
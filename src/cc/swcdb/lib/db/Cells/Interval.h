/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_Interval_h
#define swcdb_db_Cells_Interval_h

#include "swcdb/lib/db/Cells/Cell.h"
#include "SpecsInterval.h"


namespace SWC {  namespace DB { namespace Cells {

class Interval {

  /* encoded-format: 
      key_begin-encoded key_end-encoded vi64(ts_earliest) vi64(ts_latest)
  */

  public:

  typedef std::shared_ptr<Interval> Ptr;

  explicit Interval() { }

  explicit Interval(const uint8_t **ptr, size_t *remain) {
    decode(ptr, remain, true); 
  }

  virtual ~Interval(){ 
    free();
  }

  void copy(Ptr other){
    m_key_begin.copy(other->get_key_begin());
    m_key_end.copy(other->get_key_end());

    m_ts_earliest.copy(other->get_ts_earliest());
    m_ts_latest.copy(other->get_ts_latest());
  }

  void free(){
    m_key_begin.free();
    m_key_end.free();
  }

  const Specs::Key& get_key_begin(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_begin;
  }
  const Specs::Key& get_key_end(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_end;
  }
  const Specs::Timestamp& get_ts_earliest(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ts_earliest;
  }
  const Specs::Timestamp& get_ts_latest(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ts_latest;
  }

  void key_begin(const Specs::Key& key){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_key_begin.copy(key);
  }
  void key_end(const Specs::Key& key){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_key_end.copy(key);
  }
  void ts_earliest(const Specs::Timestamp& ts){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ts_earliest.copy(ts);
  }
  void ts_latest(const Specs::Timestamp& ts){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ts_latest.copy(ts);
  }

  void expand(const Ptr& other, bool initiated){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto& key_begin = other->get_key_begin();
    if(!initiated || !m_key_begin.is_matching(key_begin)) {
      m_key_begin.copy(key_begin);
      m_key_begin.set(-1, Condition::GE);
    }
    auto& key_end = other->get_key_end();
    if(!initiated || !m_key_end.is_matching(key_end)) {
      m_key_end.copy(key_end);
      m_key_end.set(-1, Condition::LE);
    }
    
    auto& earliest = other->get_ts_earliest();
    if(!initiated || !m_ts_earliest.is_matching(earliest.value)) {
      m_ts_earliest.value = earliest.value;
      m_ts_earliest.comp = Condition::GE;
    }
    auto& latest = other->get_ts_latest();
    if(!initiated || !m_ts_latest.is_matching(latest.value)) {
      m_ts_latest.value = latest.value;
      m_ts_latest.comp = Condition::LE;
    }
  }
  
  bool equal(const Ptr &other){
    std::lock_guard<std::mutex> lock(m_mutex);
    return
      other != nullptr &&
      m_key_begin.equal(other->get_key_begin()) && 
      m_key_end.equal(other->get_key_end()) && 
      m_ts_earliest.equal(other->get_ts_earliest()) && 
      m_ts_latest.equal(other->get_ts_latest());
  }

  bool is_in_begin(const Specs::Key &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_begin.is_matching(key);
  }
  
  bool is_in_end(const Specs::Key &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_end.is_matching(key);
  }
  
  bool consist(const Ptr& other) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_begin.is_matching(other->get_key_end()) && 
           m_key_end.is_matching(other->get_key_begin());
  }

  bool includes(const Specs::Interval::Ptr interval) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_end.empty() || m_key_begin.empty() ||
           interval->key_start.empty() || interval->key_finish.empty() ||
           (
            m_key_begin.is_matching(interval->key_finish) 
            &&
            m_key_end.is_matching(interval->key_start)
           );
           
  }

  bool consist(const DB::Cell::Key& key){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_key_begin.is_matching(key) && m_key_end.is_matching(key);
  }

  const size_t encoded_length(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return  m_key_begin.encoded_length()
          + m_key_end.encoded_length()
          + m_ts_earliest.encoded_length()
          + m_ts_latest.encoded_length();
  }

  void encode(uint8_t **ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_key_begin.encode(ptr);
    m_key_end.encode(ptr);
    m_ts_earliest.encode(ptr);
    m_ts_latest.encode(ptr);
  }

  void decode(const uint8_t **ptr, size_t *remain, bool owner=false){
    m_key_begin.decode(ptr, remain, owner);
    m_key_end.decode(ptr, remain, owner);
    m_ts_earliest.decode(ptr, remain);
    m_ts_latest.decode(ptr, remain);
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("Interval(begin=");
    s.append(m_key_begin.to_string());
    s.append( " end=");
    s.append(m_key_end.to_string());
    s.append( " earliest=");
    s.append(m_ts_earliest.to_string());
    s.append( " latest=");
    s.append(m_ts_latest.to_string());
    return s;
  }

  private:

  std::mutex        m_mutex;

  Specs::Key        m_key_begin;
  Specs::Key        m_key_end;  
  Specs::Timestamp  m_ts_earliest;
  Specs::Timestamp  m_ts_latest;

};

}}}


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

inline static  ArrChain<Cells::Interval::Ptr> ranggsfgfghes;
*/

#endif
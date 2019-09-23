/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Range_h
#define swcdb_lib_db_Columns_MNGR_Range_h

#include <random>

#include "swcdb/lib/db/Columns/RangeBase.h"



namespace SWC { namespace server { namespace Mngr {

class Range;
typedef std::shared_ptr<Range> RangePtr;

class Range : public DB::RangeBase {
  public:
  
  enum State {
    NOTSET,
    DELETED,
    ASSIGNED,
    QUEUED
  };

  RangePtr static Ptr(const DB::RangeBasePtr& other){
    return std::dynamic_pointer_cast<Range>(other);
  }

  Range(int64_t cid, int64_t rid)
        : RangeBase(cid, rid), 
          m_state(State::NOTSET), rs_id(0), m_last_rs(nullptr) { 
  }

  void init(int &err){

  }

  virtual ~Range(){}
  
  bool deleted(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::DELETED;
  }

  bool assigned(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::ASSIGNED;
  }

  bool queued(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::QUEUED;
  }

  bool need_assign(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::NOTSET;
  }

  void set_state(State new_state, uint64_t new_rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = new_state;
    rs_id = new_rs_id;
  }
  
  void set_deleted(){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = State::DELETED;
  }

  uint64_t get_rs_id(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return rs_id;
  }

  void set_rs_id(uint64_t new_rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);
    rs_id = new_rs_id;
  }

  Files::RsDataPtr get_last_rs(int &err){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_last_rs == nullptr)
      m_last_rs = DB::RangeBase::get_last_rs(err);
    return m_last_rs;
  }
  
  void clear_last_rs(){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_last_rs = nullptr;
  }

  void set(DB::Cells::Intervals::Ptr intervals){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_intervals == nullptr)
       m_intervals = std::make_shared<DB::Cells::Intervals>();
    m_intervals->copy(intervals);
  }

  std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("[");
    s.append(DB::RangeBase::to_string());
    s.append(", state=");
    s.append(std::to_string(m_state));
    s.append(", rs_id=");
    s.append(std::to_string(rs_id));
    s.append("]");
    return s;
  }

  
  bool chained_set(RangePtr range, RangePtr& current){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(m_intervals != nullptr 
      && !m_intervals->is_in_end(range->get_intervals()->get_key_begin())) {
      range->chained_set_next(current);
      range->chained_set_prev(m_chained_prev);
      m_chained_prev = range;
      return true;
    }

    if(m_chained_next == nullptr){
      if(range->rid != -1)
        range->chained_set_prev(current); 
      m_chained_next = range;
      return true;
    }

    current = m_chained_next;
    return false;
  }

  void chained_consist(DB::Specs::Interval& intvals, RangePtr& found,
                       DB::Specs::Key &next_key, RangePtr& current){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "chained_consist, rid=" << current->rid  
             << "\n this  " << (m_intervals==nullptr?
                                std::string("NULL"): m_intervals->to_string())
             << "\n other " << intvals.to_string();

    if(m_intervals == nullptr || !m_intervals->consist(intvals)) {
      std::cout << " FALSE\n";
      current = nullptr;
      return;
    }
    std::cout << " TRUE\n";

    if(found != nullptr)
      next_key.copy(m_intervals->get_key_begin());
    else 
      found = current;

    current = m_chained_next;
    return;
  }

  void chained_consist(const DB::Cell::Key& key, RangePtr& found,
                       DB::Specs::Key &next_key, RangePtr& current){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "chained_consist(cell-key), rid=" << current->rid  
    << (m_intervals==nullptr?
        std::string("NULL"): m_intervals->to_string()) << "\n";

    if(m_intervals == nullptr || !m_intervals->consist(key)) {
      std::cout << " FALSE\n";
      current = nullptr;
      return;
    }
    std::cout << " TRUE\n";

    if(found != nullptr)
      next_key.copy(m_intervals->get_key_begin());
    else 
      found = current;

    current = m_chained_next;
    return;
  }

  void chained_next(RangePtr& range) {
    std::lock_guard<std::mutex> lock(m_mutex);
    range = m_chained_next;
  }

  void chained_remove(){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(rid != -1)
      m_chained_prev->chained_set_next(m_chained_next);
    if(m_chained_next != nullptr)
      m_chained_next->chained_set_prev(m_chained_prev);
  }

  private:

  void chained_set_next(RangePtr next) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chained_next = next;
  }

  void chained_set_prev(RangePtr prev) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chained_prev = prev;
  }


  uint64_t          rs_id;
  State             m_state;
  Files::RsDataPtr  m_last_rs;
  
  RangePtr          m_chained_next=nullptr;
  RangePtr          m_chained_prev=nullptr;

};



}}}
#endif
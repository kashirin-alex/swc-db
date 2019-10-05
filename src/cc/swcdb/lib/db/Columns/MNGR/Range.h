/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Range_h
#define swcdb_lib_db_Columns_MNGR_Range_h

#include <random>

#include "swcdb/lib/db/Columns/RangeBase.h"



namespace SWC { namespace server { namespace Mngr {


class Range : public DB::RangeBase {
  public:

  typedef std::shared_ptr<Range> Ptr;
  
  enum State {
    NOTSET,
    DELETED,
    ASSIGNED,
    QUEUED
  };


  Range(int64_t cid, int64_t rid)
        : RangeBase(cid, rid), 
          m_state(State::NOTSET), rgr_id(0), m_last_rgr(nullptr) { 
  }

  void init(int &err){

  }

  inline Ptr shared() {
    return std::dynamic_pointer_cast<Range>(shared_from_this());
  }

  inline static Ptr shared(const DB::RangeBase::Ptr& other){
    return std::dynamic_pointer_cast<Range>(other);
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

  void set_state(State new_state, uint64_t new_rgr_id){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = new_state;
    rgr_id = new_rgr_id;
  }
  
  void set_deleted(){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = State::DELETED;
  }

  uint64_t get_rgr_id(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return rgr_id;
  }

  void set_rgr_id(uint64_t new_rgr_id){
    std::lock_guard<std::mutex> lock(m_mutex);
    rgr_id = new_rgr_id;
  }

  Files::RgrDataPtr get_last_rgr(int &err){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_last_rgr == nullptr)
      m_last_rgr = DB::RangeBase::get_last_rgr(err);
    return m_last_rgr;
  }
  
  void clear_last_rgr(){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_last_rgr = nullptr;
  }

  void set(DB::Cells::Interval::Ptr interval){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_interval == nullptr)
       m_interval = std::make_shared<DB::Cells::Interval>();
    m_interval->copy(interval);
  }

  std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("[");
    s.append(DB::RangeBase::to_string());
    s.append(", state=");
    s.append(std::to_string(m_state));
    s.append(", rgr=");
    s.append(std::to_string(rgr_id));
    s.append("]");
    return s;
  }

  
  bool chained_set(Ptr range, Ptr& current){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(m_interval != nullptr) {
      DB::Cells::Interval::Ptr intval = range->get_interval();
      if(intval != nullptr 
        && !m_interval->is_in_end(intval->get_key_begin())) {
        range->chained_set_next(current);
        if(m_chained_prev != nullptr) {
          range->chained_set_prev(m_chained_prev);
          m_chained_prev->chained_set_next(range);
        }
        m_chained_prev = range;
        return true;
      }
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

  void chained_consist(DB::Specs::Interval::Ptr& intval, Ptr& found,
                       DB::Specs::Key &next_key, Ptr& current){
    std::lock_guard<std::mutex> lock(m_mutex);


    if(m_interval == nullptr || !m_interval->includes(intval)) {
      std::cout << " FALSE chained_consist, rid=" << current->rid
                << "\n this  " << m_interval->to_string()
                << "\n other " << intval->to_string();
      current = nullptr;
      return;
    }

    if(found != nullptr)
      next_key.copy(m_interval->get_key_begin());
    else 
      found = current;

    current = m_chained_next;
    return;
  }

  void chained_next(Ptr& range) {
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

  void chained_set_next(Ptr next) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chained_next = next;
  }

  void chained_set_prev(Ptr prev) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chained_prev = prev;
  }


  uint64_t            rgr_id;
  State               m_state;
  Files::RgrDataPtr   m_last_rgr;
  
  Ptr                 m_chained_next=nullptr;
  Ptr                 m_chained_prev=nullptr;

};



}}}
#endif
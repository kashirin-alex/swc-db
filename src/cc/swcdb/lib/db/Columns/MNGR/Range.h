/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Range_h
#define swcdb_lib_db_Columns_MNGR_Range_h

#include <random>

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Protocol/req/UnloadRange.h"



namespace SWC { namespace server { namespace Mngr {


class Range : public DB::RangeBase {
  public:
  
  enum State {
    NOTSET,
    DELETED,
    ASSIGNED,
    QUEUED
  };

  Range(int64_t cid, int64_t rid)
        : RangeBase(cid, rid), 
          m_state(State::NOTSET), rs_id(0) { 
  }

  void init(){

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
    m_state = Range::State::DELETED;
  }

  uint64_t get_rs_id(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return rs_id;
  }

  void set_rs_id(uint64_t new_rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);
    rs_id = new_rs_id;
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

  private:
  State     m_state;
  uint64_t  rs_id;
};

typedef std::shared_ptr<Range> RangePtr;


}}}
#endif
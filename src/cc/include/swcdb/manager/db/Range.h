/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_manager_db_Range_h
#define swc_manager_db_Range_h

#include <shared_mutex>

#include "swcdb/common/Files/RgrData.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Cells/Interval.h"

#include "swcdb/db/Columns/RangeBase.h"


namespace SWC { namespace Manager {

class Range final {
  public:

  typedef std::shared_ptr<Range> Ptr;

  enum State {
    NOTSET,
    DELETED,
    ASSIGNED,
    CREATED,
    QUEUED
  };

  const ColumnCfg*  cfg;
  const int64_t     rid;

  Range(const ColumnCfg* cfg, const int64_t rid)
        : cfg(cfg), rid(rid), m_path(DB::RangeBase::get_path(cfg->cid, rid)),
          m_state(State::NOTSET), m_rgr_id(0), m_last_rgr(nullptr) { 
  }

  void init(int &err) { }

  virtual ~Range() { }
  
  bool deleted() {
    std::shared_lock lock(m_mutex);
    return m_state == State::DELETED;
  }

  bool assigned() {
    std::shared_lock lock(m_mutex);
    return m_state == State::ASSIGNED;
  }

  bool queued() {
    std::shared_lock lock(m_mutex);
    return m_state == State::QUEUED;
  }

  bool need_assign() {
    std::shared_lock lock(m_mutex);
    return m_state == State::NOTSET;
  }

  void set_state(State new_state, uint64_t rgr_id) {
    std::scoped_lock lock(m_mutex);
    m_state = new_state;
    m_rgr_id = rgr_id;
  }
  
  void set_deleted() {
    std::scoped_lock lock(m_mutex);
    m_state = State::DELETED;
  }

  uint64_t get_rgr_id() {
    std::shared_lock lock(m_mutex);
    return m_rgr_id;
  }

  void set_rgr_id(uint64_t rgr_id) {
    std::scoped_lock lock(m_mutex);
    m_rgr_id = rgr_id;
  }

  Files::RgrData::Ptr get_last_rgr(int &err) {
    std::scoped_lock lock(m_mutex);
    if(m_last_rgr == nullptr)
      m_last_rgr = Files::RgrData::get_rgr(
        err, DB::RangeBase::get_path_ranger(m_path));
    return m_last_rgr;
  }

  void clear_last_rgr() {
    std::scoped_lock lock(m_mutex);
    m_last_rgr = nullptr;
  }

  void set(const DB::Cells::Interval& intval) {
    std::scoped_lock lock(m_mutex);
    m_key_begin.copy(intval.key_begin);
    m_key_end.copy(intval.key_end);
  }

  void get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end) {
    std::shared_lock lock(m_mutex);
    key_begin.copy(m_key_begin);
    key_end.copy(m_key_end);
  }

  const bool equal(const DB::Cells::Interval& intval) {
    std::shared_lock lock(m_mutex);
    return m_key_begin.equal(intval.key_begin);
           m_key_end.equal(intval.key_end);
  }

  bool includes(const DB::Cell::Key& range_begin, 
                const DB::Cell::Key& range_end, uint32_t any_is=0) {
    std::shared_lock lock(m_mutex);
    return (
        m_key_begin.empty() || 
        m_key_begin.count == any_is ||
        range_end.empty() || 
        range_end.compare(m_key_begin) != Condition::GT
      ) && (
        m_key_end.empty() || 
        m_key_end.count == any_is ||
        range_begin.empty() || 
        range_begin.compare(m_key_end) != Condition::LT
      );
  }

  const bool after(const Ptr& range) {
    std::shared_lock lock(m_mutex);
    return range->before(m_key_end);
  }

  const bool before(const DB::Cell::Key& key) {
    std::shared_lock lock(m_mutex);
    return m_key_end.empty() || 
          (!key.empty() &&  key.compare(m_key_end) != Condition::GT);
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);
    std::string s("[");
    s.append(cfg->to_string());
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" state=");
    s.append(std::to_string(m_state));
    s.append(" rgr=");
    s.append(std::to_string(m_rgr_id));
    s.append("]");
    return s;
  }

  private:
  const std::string     m_path;

  std::shared_mutex     m_mutex;
  uint64_t              m_rgr_id;
  State                 m_state;
  Files::RgrData::Ptr   m_last_rgr;

  DB::Cell::Key         m_key_begin;
  DB::Cell::Key         m_key_end;

};



}}
#endif // swc_manager_db_Range_h
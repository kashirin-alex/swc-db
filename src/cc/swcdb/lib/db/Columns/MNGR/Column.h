/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Column_h
#define swcdb_lib_db_Columns_MNGR_Column_h

#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>

namespace SWC { namespace server { namespace Mngr {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  enum State {
    OK,
    DELETED
  };
  
  static bool create(int &err, int64_t id) {
    Env::FsInterface::interface()->mkdirs(err, Range::get_column_path(id));
    return true;
  }

  static bool remove(int &err, int64_t id) {
    Env::FsInterface::interface()->rmdir(err, Range::get_column_path(id));
    return true;
  }

  Column(int64_t id) : cid(id), m_ranges(std::make_shared<RangesMap>()) { }

  virtual ~Column(){}

  void init(int &err) {
    FS::IdEntries_t entries;

    {
      std::lock_guard<std::mutex> lock(m_mutex);

      if(!exists_range_path(err))
        create_range_path(err);
      else
        ranges_by_fs(err, entries); 
    }

    if(entries.empty())
      entries.push_back(1); // initialize 1st range

    for(auto rid : entries)
      get_range(err, rid, true); 

    m_state = State::OK;
  }

  RangePtr get_range(int &err, int64_t rid, bool initialize=false){
    RangePtr range = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      auto it = m_ranges->find(rid);
      if (it != m_ranges->end())
        return it->second;
      else if(initialize) {
        range = std::make_shared<Range>(cid, rid);
        m_ranges->insert(RangesMapPair(rid, range));;
      }
    }
    if(initialize)
      range->init(err);
    return range;
  }
  
  RangePtr get_next_unassigned(){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      if(it->second->need_assign())
        return it->second;
    }
    return nullptr;
  }

  void set_rs_unassigned(uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      if(it->second->get_rs_id() == rs_id){
        it->second->set_state(Range::State::NOTSET, 0);
      }
    }
  }

  void change_rs(uint64_t rs_id_old, uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      if(it->second->get_rs_id() == rs_id_old)
        it->second->set_rs_id(rs_id);
    }
  }

  int64_t get_next_rid(){
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t rid = 0;
    for(;;){
      auto it = m_ranges->find(++rid);
      if(it == m_ranges->end())
        break;
      if(it->second->deleted())
        break;
    }
    return rid;
  }
  
  void assigned(std::vector<uint64_t> &rs_ids){
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t rs_id;
    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      rs_id = it->second->get_rs_id();
      if(rs_id == 0)
        continue;
      if(std::find_if(rs_ids.begin(), rs_ids.end(), [rs_id]
      (const uint64_t& rs_id2){return rs_id == rs_id2;}) == rs_ids.end())
        rs_ids.push_back(rs_id);
    }
  }
  
  bool do_remove(){
    bool was;
    std::lock_guard<std::mutex> lock(m_mutex);
    was = m_state == State::DELETED;
    if(!was){
      m_state = State::DELETED;
      for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it)
        it->second->set_deleted();
    }
    return !was;
  }

  bool finalize_remove(int &err, uint64_t rs_id=0){
    bool empty;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      for(;;){
        auto it = m_ranges->begin();
        if(it == m_ranges->end())
          break;
        if(rs_id == 0 
          || it->second->get_rs_id() == rs_id
          || it->second->get_rs_id() == 0)
          m_ranges->erase(it);
      }
      empty = m_ranges->empty();
      if(empty)
        Env::FsInterface::interface()->rmdir(err, Range::get_path(cid));
    }

    if(empty)
      HT_DEBUGF("FINALIZED REMOVE %s", to_string().c_str());
    return empty;
  }

  std::string to_string(){
    std::string s("[cid=");
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      s.append(std::to_string(cid));
    }

    s.append(", next-rid=");
    s.append(std::to_string(get_next_rid()));
    s.append(", ranges=(");
    
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
        s.append(it->second->to_string());
        s.append(",");
      }
    }
    s.append(")]");
    return s;
  }

  private:
  
  bool exists(int &err) {
    return Env::FsInterface::interface()->exists(err, Range::get_column_path(cid));
  }

  bool exists_range_path(int &err) {
    return Env::FsInterface::interface()->exists(err, Range::get_path(cid));
  }

  void create_range_path(int &err) {
    Env::FsInterface::interface()->mkdirs(err, Range::get_path(cid));
  }

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    Env::FsInterface::interface()->get_structured_ids(err, Range::get_path(cid), entries);
  }

  std::mutex                 m_mutex;
  int64_t                    cid;
  std::shared_ptr<RangesMap> m_ranges;
  State                      m_state;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif
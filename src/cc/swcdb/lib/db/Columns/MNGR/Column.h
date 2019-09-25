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


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  enum State {
    OK,
    LOADING,
    DELETED
  };
  
  static bool create(int &err, int64_t id) {
    Env::FsInterface::interface()->mkdirs(err, Range::get_column_path(id));
    return true;
  }

  static bool remove(int &err, int64_t id) {
    Env::FsInterface::interface()->rmdir_incl_opt_subs(
      err, Range::get_column_path(id), Range::get_column_path());
    return true;
  }

  Column(int64_t id) 
        : cid(id), m_state(State::LOADING),
          m_base_range(std::make_shared<Range>(cid, -1)) {
  }

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
  }

  void set_loading(){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_state == State::OK)
      m_state = State::LOADING;
  }

  void state(int& err) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_state == State::OK)
      return;
    if(m_state == State::DELETED)
      err = Error::COLUMN_MARKED_REMOVED;
    else
      err = Error::COLUMN_NOT_READY;
      
  }
  
  RangePtr get_range(int &err, int64_t rid, bool initialize=false){
    RangePtr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      if(range->rid == rid)
        return range;
    }
    if(initialize) {
      range = std::make_shared<Range>(cid, rid);
      range->init(err);
      chained_set(range);
      return range;
    } else
      return nullptr;
  }

  RangePtr get_range(int &err, DB::Specs::Interval::Ptr& intervals, 
                     DB::Specs::Key &next_key){
    RangePtr found;
    RangePtr range = m_base_range;
    range->chained_next(range);
    for(;;){
      range->chained_consist(intervals, found, next_key, range);
      if(range == nullptr || !next_key.empty())
        break;
    }
    return found;
  }

  void chained_set(RangePtr& range, DB::Cells::Intervals::Ptr& intervals){
    DB::Cells::Intervals::Ptr intvals = range->get_intervals();
    if((intvals == nullptr && intervals == nullptr) || 
        (intervals != nullptr && intervals->equal(intvals)))
      return;
    range->chained_remove();
    range->set(intervals);
    chained_set(range);
    if(!range->assigned())
      return;
    
    RangePtr next = m_base_range;
    do{
      next->chained_next(next);
    } while(next != nullptr && next->assigned());
    if(next == nullptr) {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::LOADING)
        m_state = State::OK;
    }
  }

  int64_t get_next_rid(){
    int64_t rid = 0;
    RangePtr range;
    for(;;){
      rid++;
      range = m_base_range;
      for(;;){
        range->chained_next(range);
        if(range == nullptr || range->rid == rid)
          break;
      }
      if(range == nullptr)
        break;
    }
    return rid;
  }

  RangePtr get_next_unassigned(){    
    RangePtr range = m_base_range;
    do{
      range->chained_next(range);
    } while(range != nullptr && !range->need_assign());
    
    if(range != nullptr)
      set_loading();
    return range;
  }

  void set_rs_unassigned(uint64_t rs_id){

    RangePtr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      if(range->get_rs_id() == rs_id) {
        range->set_state(Range::State::NOTSET, 0);
        set_loading();
      }
    }
    
    remove_rs_schema(rs_id);
  }

  void change_rs(uint64_t rs_id_old, uint64_t rs_id){

    RangePtr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      if(range->get_rs_id() == rs_id_old)
        range->set_rs_id(rs_id);
    }

    std::lock_guard<std::mutex> lock(m_mutex);    
    for(auto it = m_schemas_rev.begin(); it != m_schemas_rev.end(); ++it){
      if(it->first == rs_id_old) {
        m_schemas_rev.insert(RsSchemaRev(rs_id, it->second));;
        m_schemas_rev.erase(it);
        break;
      }
    }
  }

  void change_rs_schema(uint64_t rs_id, int64_t rev=0){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_schemas_rev.find(rs_id);
    if(it == m_schemas_rev.end())
       m_schemas_rev.insert(RsSchemaRev(rs_id, rev));
    else
      it->second = rev;
  }

  void remove_rs_schema(uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_schemas_rev.find(rs_id);
    if(it != m_schemas_rev.end())
       m_schemas_rev.erase(it);
  }

  void need_schema_sync(int64_t rev, std::vector<uint64_t> &rs_ids){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_schemas_rev.begin(); it != m_schemas_rev.end(); ++it){
      if(it->second != rev)
        rs_ids.push_back(it->first);
    }
  }

  bool need_schema_sync(uint64_t rs_id, int64_t rev){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_schemas_rev.find(rs_id);
    if(it != m_schemas_rev.end())
      return rev != it->second;
    return true;
  }
  
  void assigned(std::vector<uint64_t> &rs_ids){
    RangePtr range = m_base_range;
    uint64_t rs_id;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      rs_id = range->get_rs_id();
      if(rs_id == 0)
        continue;
      if(std::find_if(rs_ids.begin(), rs_ids.end(), [rs_id]
      (const uint64_t& rs_id2){return rs_id == rs_id2;}) == rs_ids.end())
        rs_ids.push_back(rs_id);
    }
  }
  
  bool do_remove(){
    bool was;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      was = m_state == State::DELETED;
      m_state = State::DELETED;
      m_schemas_rev.clear();
    }
    if(!was){
      RangePtr range = m_base_range;
      for(;;){
        range->chained_next(range);
        if(range == nullptr)
          break;
        range->set_deleted();
      }
    }
    return !was;
  }

  bool finalize_remove(int &err, uint64_t rs_id=0){
    uint64_t id;
    RangePtr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      id = range->get_rs_id();
      if(rs_id == 0 || id == rs_id || id == 0)
        range->chained_remove();
    }

    range = m_base_range;
    range->chained_next(range);
    bool empty = range == nullptr;
    if(empty) {
      Env::FsInterface::interface()->rmdir(err, Range::get_path(cid));
      HT_DEBUGF("FINALIZED REMOVE %s", to_string().c_str());
    }
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
    
    RangePtr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      s.append(range->to_string());
      s.append(",");
    }
    s.append(")]");
    return s;
  }

  private:
  
  bool exists(int &err) {
    return Env::FsInterface::interface()->exists(
      err, Range::get_column_path(cid));
  }

  bool exists_range_path(int &err) {
    return Env::FsInterface::interface()->exists(
      err, Range::get_path(cid));
  }

  void create_range_path(int &err) {
    Env::FsInterface::interface()->mkdirs(
      err, Range::get_path(cid));
  }

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    Env::FsInterface::interface()->get_structured_ids(
      err, Range::get_path(cid), entries);
  }

  
  void chained_set(RangePtr range){
    RangePtr current = m_base_range;
    while(!current->chained_set(range, current));
  }


  std::mutex                 m_mutex;
  int64_t                    cid;
  std::atomic<State>         m_state;
  const RangePtr             m_base_range;


  typedef std::pair<uint64_t, int64_t>    RsSchemaRev;
  std::unordered_map<uint64_t, int64_t>   m_schemas_rev;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif
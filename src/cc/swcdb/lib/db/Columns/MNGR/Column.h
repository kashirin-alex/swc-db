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
  
  static bool create(int &err, const int64_t id) {
    Env::FsInterface::interface()->mkdirs(err, Range::get_column_path(id));
    return true;
  }

  static bool remove(int &err, const int64_t id) {
    Env::FsInterface::interface()->rmdir_incl_opt_subs(
      err, Range::get_column_path(id), Range::get_column_path());
    return true;
  }

  Column(const int64_t id) 
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

  void set_loading() {
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
  
  Range::Ptr get_range(int &err, const int64_t rid, bool initialize=false) {
    Range::Ptr range = m_base_range;
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

  Range::Ptr get_range(int &err, DB::Specs::Interval::Ptr interval, 
                       DB::Specs::Key &key_next) {
    Range::Ptr found;
    Range::Ptr range = m_base_range;
    range->chained_next(range);
    for(;;){
      range->chained_consist(interval, found, key_next, range);
      if(range == nullptr || !key_next.empty())
        break;
    }
    return found;
  }

  void chained_set(Range::Ptr& range, const DB::Cells::Interval& interval) {
    const DB::Cells::Interval& intval = range->get_interval();

    if(!intval.was_set && !interval.was_set) 
      return;

    if(!intval.was_set || !interval.equal(intval)) {
      range->chained_remove();
      range->set(interval);
      chained_set(range);
      if(!range->assigned())
        return;
    }

    Range::Ptr next = m_base_range;
    do{
      next->chained_next(next);
    } while(next != nullptr && next->assigned());
    if(next == nullptr) {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::LOADING)
        m_state = State::OK;
    }
  }

  int64_t get_next_rid() {
    int64_t rid = 0;
    Range::Ptr range;
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

  Range::Ptr get_next_unassigned() {    
    Range::Ptr range = m_base_range;
    do{
      range->chained_next(range);
    } while(range != nullptr && !range->need_assign());
    
    if(range != nullptr)
      set_loading();
    return range;
  }

  void set_rgr_unassigned(uint64_t id) {

    Range::Ptr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      if(range->get_rgr_id() == id) {
        range->set_state(Range::State::NOTSET, 0);
        set_loading();
      }
    }
    
    remove_rgr_schema(id);
  }

  void change_rgr(uint64_t rgr_id_old, uint64_t id) {
    Range::Ptr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      if(range->get_rgr_id() == rgr_id_old)
        range->set_rgr_id(id);
    }

    std::lock_guard<std::mutex> lock(m_mutex);    
    for(auto it = m_schemas_rev.begin(); it != m_schemas_rev.end(); ++it){
      if(it->first == rgr_id_old) {
        m_schemas_rev.insert(RsSchemaRev(id, it->second));;
        m_schemas_rev.erase(it);
        break;
      }
    }
  }

  void change_rgr_schema(const uint64_t id, int64_t rev=0) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_schemas_rev.find(id);
    if(it == m_schemas_rev.end())
       m_schemas_rev.insert(RsSchemaRev(id, rev));
    else
      it->second = rev;
  }

  void remove_rgr_schema(const uint64_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_schemas_rev.find(id);
    if(it != m_schemas_rev.end())
       m_schemas_rev.erase(it);
  }

  void need_schema_sync(int64_t rev, std::vector<uint64_t> &rgr_ids) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_schemas_rev.begin(); it != m_schemas_rev.end(); ++it){
      if(it->second != rev)
        rgr_ids.push_back(it->first);
    }
  }

  bool need_schema_sync(const uint64_t id, int64_t rev) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_schemas_rev.find(id);
    if(it != m_schemas_rev.end())
      return rev != it->second;
    return true;
  }
  
  void assigned(std::vector<uint64_t> &rgr_ids) {
    Range::Ptr range = m_base_range;
    uint64_t id;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      id = range->get_rgr_id();
      if(id == 0)
        continue;
      if(std::find_if(rgr_ids.begin(), rgr_ids.end(), [id]
      (const uint64_t& rgr_id2){return id == rgr_id2;}) == rgr_ids.end())
        rgr_ids.push_back(id);
    }
  }
  
  bool do_remove() {
    bool was;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      was = m_state == State::DELETED;
      m_state = State::DELETED;
      m_schemas_rev.clear();
    }
    if(!was){
      Range::Ptr range = m_base_range;
      for(;;){
        range->chained_next(range);
        if(range == nullptr)
          break;
        range->set_deleted();
      }
    }
    return !was;
  }

  bool finalize_remove(int &err, uint64_t id=0) {
    uint64_t eid;
    Range::Ptr range = m_base_range;
    for(;;){
      range->chained_next(range);
      if(range == nullptr)
        break;
      eid = range->get_rgr_id();
      if(id == 0 || eid == id || eid == 0)
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

  const std::string to_string() {
    std::string s("[cid=");
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      s.append(std::to_string(cid));
    }

    s.append(", next-rid=");
    s.append(std::to_string(get_next_rid()));
    s.append(", ranges=(");
    
    Range::Ptr range = m_base_range;
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

  void chained_set(Range::Ptr range){
    Range::Ptr current = m_base_range;
    while(!current->chained_set(range, current));
  }


  std::mutex                 m_mutex;
  const int64_t              cid;
  std::atomic<State>         m_state;
  const Range::Ptr           m_base_range;


  typedef std::pair<uint64_t, int64_t>    RsSchemaRev;
  std::unordered_map<uint64_t, int64_t>   m_schemas_rev;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif
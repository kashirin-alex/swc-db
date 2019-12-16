/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Column_h
#define swcdb_lib_db_Columns_MNGR_Column_h

#include "swcdb/db/Columns/Mngr/Range.h"

#include <memory>
#include <unordered_map>

namespace SWC { namespace server { namespace Mngr {


class Column final {
  
  public:

  enum State {
    OK,
    LOADING,
    DELETED
  };
  typedef std::shared_ptr<Column> Ptr;
  
  static bool create(int &err, const int64_t id) {
    Env::FsInterface::interface()->mkdirs(err, Range::get_column_path(id));
    return true;
  }

  static bool remove(int &err, const int64_t id) {
    Env::FsInterface::interface()->rmdir_incl_opt_subs(
      err, Range::get_column_path(id), Range::get_column_path());
    return true;
  }

  const DB::ColumnCfg  cfg;

  Column(const int64_t cid) : cfg(cid), m_state(State::LOADING) {
  }

  virtual ~Column(){}

  void init(int &err) {
    FS::IdEntries_t entries;

    {
      std::scoped_lock lock(m_mutex);

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
    std::scoped_lock lock(m_mutex);
    _set_loading();
  }

  void state(int& err) {
    std::shared_lock lock(m_mutex);
    if(m_state == State::OK)
      return;
    if(m_state == State::DELETED)
      err = Error::COLUMN_MARKED_REMOVED;
    else
      err = Error::COLUMN_NOT_READY;
  }
  
  Range::Ptr get_range(int &err, const int64_t rid, bool initialize=false) {
    std::scoped_lock lock(m_mutex);

    auto it = std::find_if(m_ranges.begin(), m_ranges.end(), 
                          [rid](const Range::Ptr& range) 
                          {return range->rid == rid;});
    if(it != m_ranges.end())
      return *it;

    if(initialize) {
      Range::Ptr range = std::make_shared<Range>(&cfg, rid);
      //range->init(err);
      m_ranges.push_back(range);
      return range;
    } 
    return nullptr;
  }

  Range::Ptr get_range(int &err, const DB::Specs::Interval& intval, 
                       bool &next_key) {
    std::shared_lock lock(m_mutex);
    int64_t offset = intval.flags.offset;

    Range::Ptr found = nullptr;
    for(auto& range : m_ranges){
      if(range->includes(intval)) {
        if(offset) {
          --offset;
          continue;
        }
        if(found == nullptr){
          found = range;
          continue;
        }
        next_key = true;
        break;
      }
    }
    return found;
  }

  void sort(Range::Ptr& range, const DB::Cells::Interval& interval) {
    std::scoped_lock lock(m_mutex);
    
    // std::sort(m_ranges.begin(), m_ranges.end(), Range); 
    if(interval.was_set != range->interval() || !range->equal(interval)) {
      range->set(interval);
      
      if(m_ranges.size() > 1) {
        m_ranges.erase(
          std::find_if(m_ranges.begin(), m_ranges.end(), 
            [rid=range->rid](const Range::Ptr& range) 
            {return range->rid == rid;})
        );
        bool added = false;
        for(auto it=m_ranges.begin();it<m_ranges.end();it++){
          if((*it)->after(range)) {
            m_ranges.insert(it, range);
            added = true;
            break;
          }
        }
        if(!added)
          m_ranges.push_back(range);
      }
    }


    if(m_state != State::LOADING)
      return;

    for(auto& range : m_ranges){
      if(!range->assigned())
        return;
    }
    if(m_state == State::LOADING)
      m_state = State::OK;
  }

  int64_t get_next_rid() {
    std::shared_lock lock(m_mutex);
    return _get_next_rid();
  }

  Range::Ptr get_next_unassigned() {
    std::scoped_lock lock(m_mutex);

    for(auto range : m_ranges) {
      if(range->need_assign()) {
        _set_loading();
        return range;
      }
    }
    return nullptr;
  }

  void set_rgr_unassigned(uint64_t id) {
    std::scoped_lock lock(m_mutex);    

    for(auto range : m_ranges){
      if(range->get_rgr_id() == id){
        range->set_state(Range::State::NOTSET, 0);
        _set_loading();
      }
    }
    _remove_rgr_schema(id);
  }

  void change_rgr(uint64_t rgr_id_old, uint64_t id) {
    std::scoped_lock lock(m_mutex);    

    for(auto range : m_ranges) {
      if(range->get_rgr_id() == rgr_id_old)
        range->set_rgr_id(id);
    }

    for(auto it = m_schemas_rev.begin(); it != m_schemas_rev.end(); ++it){
      if(it->first == rgr_id_old) {
        m_schemas_rev.insert(RsSchemaRev(id, it->second));;
        m_schemas_rev.erase(it);
        break;
      }
    }
  }

  void change_rgr_schema(const uint64_t id, int64_t rev=0) {
    std::scoped_lock lock(m_mutex);

    auto it = m_schemas_rev.find(id);
    if(it == m_schemas_rev.end())
       m_schemas_rev.insert(RsSchemaRev(id, rev));
    else
      it->second = rev;
  }

  void remove_rgr_schema(const uint64_t id) {
    std::scoped_lock lock(m_mutex);
    _remove_rgr_schema(id);
  }

  void need_schema_sync(int64_t rev, std::vector<uint64_t> &rgr_ids) {
    std::shared_lock lock(m_mutex);

    for(auto it = m_schemas_rev.begin(); it != m_schemas_rev.end(); ++it){
      if(it->second != rev)
        rgr_ids.push_back(it->first);
    }
  }

  bool need_schema_sync(const uint64_t id, int64_t rev) {
    std::shared_lock lock(m_mutex);

    auto it = m_schemas_rev.find(id);
    if(it != m_schemas_rev.end())
      return rev != it->second;
    return true;
  }
  
  void assigned(std::vector<uint64_t> &rgr_ids) {
    std::shared_lock lock(m_mutex);

    uint64_t id;
    for(auto range : m_ranges) {
      id = range->get_rgr_id();
      if(!id)
        continue;
      if(std::find_if(rgr_ids.begin(), rgr_ids.end(), 
        [id](const uint64_t& rgr_id2){return id == rgr_id2;}) == rgr_ids.end())
        rgr_ids.push_back(id);
    }
  }
  
  bool do_remove() {
    std::scoped_lock lock(m_mutex);

    bool was = m_state == State::DELETED;
    m_state = State::DELETED;
    m_schemas_rev.clear();
    
    if(!was){
      for(auto range : m_ranges)
        range->set_deleted();
    }
    return !was;
  }

  bool finalize_remove(int &err, uint64_t id=0) {
    std::scoped_lock lock(m_mutex);
    
    if(!id) {
      m_ranges.clear();
    } else {
      uint64_t eid;
      for(auto it=m_ranges.begin();it<m_ranges.end();){
        eid = (*it)->get_rgr_id();
        if(eid == id || !eid)
          m_ranges.erase(it);
        else
          it++;
      }
    }
    if(m_ranges.empty()) {
      Env::FsInterface::interface()->rmdir(err, Range::get_path(cfg.cid));
      SWC_LOGF(LOG_DEBUG, "FINALIZED REMOVE %s", _to_string().c_str());
      return true;
    }
    return false;
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);
    return _to_string();
  }

  private:
  
  bool exists(int &err) {
    return Env::FsInterface::interface()->exists(
      err, Range::get_column_path(cfg.cid));
  }

  bool exists_range_path(int &err) {
    return Env::FsInterface::interface()->exists(
      err, Range::get_path(cfg.cid));
  }

  void create_range_path(int &err) {
    Env::FsInterface::interface()->mkdirs(
      err, Range::get_path(cfg.cid));
  }

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    Env::FsInterface::interface()->get_structured_ids(
      err, Range::get_path(cfg.cid), entries);
  }

  const std::string _to_string() {
    std::string s("[");
    s.append(cfg.to_string());
    s.append(", next-rid=");
    s.append(std::to_string(_get_next_rid()));
    s.append(", ranges=(");
    
    for(auto& range : m_ranges){
      s.append(range->to_string());
      s.append(",");
    }
    s.append(")]");
    return s;
  }

  inline void _set_loading() {
    if(m_state == State::OK)
      m_state = State::LOADING;
  }
  
  int64_t _get_next_rid() {
    int64_t rid = 1;
    for(
      ;std::find_if(m_ranges.begin(), m_ranges.end(), 
        [rid](const Range::Ptr& range) 
        {return range->rid == rid;}) != m_ranges.end()
      ;rid++
    );
    return rid;
  }

  inline void _remove_rgr_schema(const uint64_t id) {
    auto it = m_schemas_rev.find(id);
    if(it != m_schemas_rev.end())
       m_schemas_rev.erase(it);
  }

  std::shared_mutex          m_mutex;
  std::atomic<State>         m_state;

  std::vector<Range::Ptr>    m_ranges;


  typedef std::pair<uint64_t, int64_t>    RsSchemaRev;
  std::unordered_map<uint64_t, int64_t>   m_schemas_rev;

};

}}}

#endif
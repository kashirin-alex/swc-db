/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Column_h
#define swcdb_lib_db_Columns_Rgr_Column_h

#include "Range.h"

#include <memory>
#include <unordered_map>

namespace SWC { namespace server { namespace Rgr {



class Column : public std::enable_shared_from_this<Column> {
  
  public:

  typedef std::shared_ptr<Column>                 Ptr;
  typedef std::unordered_map<int64_t, Range::Ptr> RangesMap;
  typedef std::pair<int64_t, Range::Ptr>          RangesMapPair;

  Column(const int64_t cid) 
        : cid(cid), m_deleting(false) { 
  }

  void init(int &err) { }

  virtual ~Column(){}

  Range::Ptr get_range(int &err, const int64_t rid, bool initialize=false) {
    Range::Ptr range = nullptr;
    {
      std::lock_guard lock(m_mutex);

      auto it = m_ranges.find(rid);
      if (it != m_ranges.end())
        return it->second;

      else if(initialize) {
        if(Env::RgrData::is_shuttingdown())
          err = Error::SERVER_SHUTTING_DOWN;
        else if(m_deleting)
          err = Error::COLUMN_MARKED_REMOVED;
        if(err)
          return range;
          
        range = std::make_shared<Range>(cid, rid);
        m_ranges.insert(RangesMapPair(rid, range));;
      }
    }
    if(initialize)
      range->init();
    return range;
  }

  void unload(const int64_t rid, Callback::RangeUnloaded_t cb) {
    Range::Ptr range = nullptr;
    {
      std::lock_guard lock(m_mutex);
      auto it = m_ranges.find(rid);
      if (it != m_ranges.end()){
        range = it->second;
        m_ranges.erase(it);
      }
    }
    if(range != nullptr)
      range->unload(cb, true);
    else
      cb(Error::OK);
  }

  void unload_all(std::atomic<int>& unloaded, Callback::RangeUnloaded_t cb) {

    for(;;){
      std::lock_guard lock(m_mutex);
      auto it = m_ranges.begin();
      if(it == m_ranges.end())
        break;
      unloaded++;
      asio::post(
        *Env::IoCtx::io()->ptr(), 
        [cb, range=it->second](){range->unload(cb, false);}
      );
      m_ranges.erase(it);
    }

    cb(Error::OK);
  }
  
  void remove_all(int &err) {
    {
      std::lock_guard lock(m_mutex);
      if(m_deleting)
        return;
      m_deleting = true;
    }
      
    for(;;){
      std::lock_guard lock(m_mutex);
      auto it = m_ranges.begin();
      if(it == m_ranges.end())
        break;
      it->second->remove(err);
      m_ranges.erase(it);
    }

    SWC_LOGF(LOG_DEBUG, "REMOVED %s", to_string().c_str());
  }

  bool removing() {
    std::shared_lock lock(m_mutex);
    return m_deleting;
  }

  Range::Ptr get_next(size_t &idx) {
    std::shared_lock lock(m_mutex);

    if(m_ranges.size() > idx){
      auto it = m_ranges.begin();
      for(int i=idx;i--;it++);
      return it->second;
    }
    idx = 0;
    return nullptr;
  }

  const size_t release(size_t bytes=0) {
    size_t released = 0;
    Range::Ptr range;
    RangesMap::iterator it;
    bool started = false;
    for(;;) {
      {
        std::shared_lock lock(m_mutex);
        if(m_deleting)
          return released;
        if(!started) { 
          it = m_ranges.begin();
          started = true;
        } else
          it++;
        if(it == m_ranges.end())
          break;
        range = it->second;
      }
      if(!range->is_loaded() || range->compacting())
        continue;
      released += range->blocks.release(bytes ? bytes-released : bytes);
      if(bytes && released >= bytes)
        break;
    }
    return released;
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);

    std::string s("[cid=");
    s.append(std::to_string(cid));

    if(m_deleting){
      s.append(", DELETING");
    }

    s.append(", ranges=(");
    
    for(auto it = m_ranges.begin(); it != m_ranges.end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")]");
    return s;
  }

  private:

  std::shared_mutex   m_mutex;
  const int64_t       cid;
  RangesMap           m_ranges;
  bool                m_deleting;
};

}}}

#endif
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_Column_h
#define swc_ranger_db_Column_h


#include "swcdb/ranger/db/Range.h"

#include <unordered_map>

namespace SWC { namespace Ranger {



class Column final : private std::unordered_map<rid_t, RangePtr> {
  
  public:

  typedef std::shared_ptr<Column>  Ptr;

  const ColumnCfg  cfg;

  Column(const cid_t cid, const DB::Schema& schema) 
        : cfg(cid, schema) { 
  }

  void init(int &err) { }

  ~Column() { }

  void schema_update(const DB::Schema& schema) {
    bool compact = cfg.c_versions > schema.cell_versions || 
                   (schema.cell_ttl && cfg.c_ttl > schema.cell_ttl);
    bool and_cells =  cfg.c_versions != schema.cell_versions ||
                      cfg.c_ttl != schema.cell_ttl ||
                      cfg.col_type != schema.col_type;
    cfg.update(schema);
    if(and_cells) {
      Mutex::scope lock(m_mutex);
      for(auto it = begin(); it != end(); ++it)
        it->second->schema_update(compact);
    }
    if(compact)
      RangerEnv::compaction_schedule(100);
  }

  void compact() {
    {
      Mutex::scope lock(m_mutex);
      for(auto it = begin(); it != end(); ++it)
        it->second->compact_require(true);
    }
    RangerEnv::compaction_schedule(100);
  }

  RangePtr get_range(int &err, const rid_t rid, bool initialize=false) {
    RangePtr range = nullptr;
    {
      Mutex::scope lock(m_mutex);

      auto it = find(rid);
      if (it != end())
        return it->second;

      else if(initialize) {
        if(RangerEnv::is_shuttingdown())
          err = Error::SERVER_SHUTTING_DOWN;
        else if(cfg.deleting)
          err = Error::COLUMN_MARKED_REMOVED;
        if(err)
          return range;
          
        emplace(rid, range = std::make_shared<Range>(&cfg, rid));
      }
    }
    if(initialize)
      range->init();
    return range;
  }

  void unload(const rid_t rid, const Callback::RangeUnloaded_t& cb) {
    RangePtr range = nullptr;
    {
      Mutex::scope lock(m_mutex);
      auto it = find(rid);
      if(it != end()) {
        range = it->second;
        erase(it);
      }
    }
    if(range)
      range->unload(cb, true);
    else
      cb(Error::OK);
  }

  void unload_all(std::atomic<int>& unloaded, 
                  const Callback::RangeUnloaded_t& cb) {
    for(iterator it;;) {
      Mutex::scope lock(m_mutex);
      if((it = begin()) == end())
        break;
      ++unloaded;
      asio::post(
        *Env::IoCtx::io()->ptr(), 
        [cb, range=it->second](){range->unload(cb, false);}
      );
      erase(it);
    }

    cb(Error::OK);
  }
  
  void remove(int &err, const rid_t rid, bool meta=true) {
    RangePtr range = nullptr;
    {
      Mutex::scope lock(m_mutex);
      auto it = find(rid);
      if (it != end()){
        range = it->second;
        erase(it);
      }
    }
    if(range)
      range->remove(err, meta);
  }

  void remove_all(int &err) {
    {
      Mutex::scope lock(m_mutex);
      if(cfg.deleting)
        return;
      cfg.deleting = true;
    }
      
    for(iterator it;;) {
      Mutex::scope lock(m_mutex);
      if((it = begin()) == end())
        break;
      it->second->remove(err);
      erase(it);
    }

    SWC_LOGF(LOG_DEBUG, "REMOVED %s", to_string().c_str());
  }

  bool removing() {
    Mutex::scope lock(m_mutex);
    return cfg.deleting;
  }

  RangePtr get_next(size_t &idx) {
    Mutex::scope lock(m_mutex);

    if(size() > idx) {
      auto it = begin();
      for(int i=idx; i; --i, ++it);
      return it->second;
    }
    idx = 0;
    return nullptr;
  }

  size_t release(size_t bytes=0) {
    size_t released = 0;
    RangePtr range;
    iterator it;
    for(size_t offset = 0; ; ++offset) {
      {
        Mutex::scope lock(m_mutex);
        if(cfg.deleting)
          return released;
        it = begin();
        for(size_t i=0; i<offset && it != end(); ++it, ++i);
        if(it == end())
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

  std::string to_string() {
    Mutex::scope lock(m_mutex);

    std::string s("[");
    s.append(cfg.to_string());

    s.append(" ranges=(");
    for(auto it = begin(); it != end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")]");
    return s;
  }

  private:

  Mutex   m_mutex;
};

}}

#endif // swc_ranger_db_Column_h
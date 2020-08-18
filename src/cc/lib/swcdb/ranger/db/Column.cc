/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/Column.h"
#include "swcdb/ranger/callbacks/ColumnsUnloaded.h"

namespace SWC { namespace Ranger {



Column::Column(const cid_t cid, const DB::Schema& schema) 
      : cfg(cid, schema), m_releasing(false) {
  RangerEnv::res().more_mem_usage(size_of());
}

void Column::init(int&) { }

Column::~Column() { 
  RangerEnv::res().less_mem_usage(size_of());
}

size_t Column::size_of() const {
  return sizeof(*this) + sizeof(Ptr);
}

size_t Column::ranges_count() {
  Mutex::scope lock(m_mutex);
  return size();
}

void Column::get_rids(std::vector<rid_t>& rids) {
  Mutex::scope lock(m_mutex);
  for(auto it = begin(); it != end(); ++it)
    rids.push_back(it->first);
}

void Column::schema_update(const DB::Schema& schema) {
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

void Column::compact() {
  {
    Mutex::scope lock(m_mutex);
    for(auto it = begin(); it != end(); ++it)
      it->second->compact_require(true);
  }
  RangerEnv::compaction_schedule(100);
}

RangePtr Column::get_range(int &err, const rid_t rid, bool initialize) {
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

void Column::unload(const rid_t rid, const Callback::RangeUnloaded_t& cb) {
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

void Column::unload_all(std::atomic<int>& unloaded, 
                        const Callback::RangeUnloaded_t& cb) {
  for(iterator it;;) {
    Mutex::scope lock(m_mutex);
    if((it = begin()) == end())
      break;
    ++unloaded;
    Env::IoCtx::post([cb, range=it->second](){range->unload(cb, false);});
    erase(it);
  }

  cb(Error::OK);
}

void Column::unload_all(const Callback::ColumnsUnloadedPtr& cb) {
  ++cb->unloading;
  for(iterator it;;) {
    Mutex::scope lock(m_mutex);
    if((it = begin()) == end())
      break;
    ++cb->unloading;
    Env::IoCtx::post([cb, range=it->second](){
      range->unload([cb, range](int err) { cb->response(err, range); }, true);
    });
    erase(it);
  }
  cb->response(Error::OK, nullptr);
}

void Column::remove(int &err, const rid_t rid, bool meta) {
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

void Column::remove_all(int &err) {
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

bool Column::removing() {
  Mutex::scope lock(m_mutex);
  return cfg.deleting;
}

RangePtr Column::get_next(size_t &idx) {
  Mutex::scope lock(m_mutex);

  if(size() > idx) {
    auto it = begin();
    for(int i=idx; i; --i, ++it);
    return it->second;
  }
  idx = 0;
  return nullptr;
}

size_t Column::release(size_t bytes) {
  size_t released = 0;
  if(m_releasing)
    return released;
  m_releasing = true;

  RangePtr range;
  iterator it;
  for(size_t offset = 0; ; ++offset) {
    {
      Mutex::scope lock(m_mutex);
      if(cfg.deleting)
        break;
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
  m_releasing = false;
  return released;
}

std::string Column::to_string() {
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

}}

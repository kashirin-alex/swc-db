/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_lib_db_Rgr_Columns_Columns_h
#define swcdb_lib_db_Rgr_Columns_Columns_h

#include "swcdb/db/Columns/Schema.h"
#include "swcdb/core/comm/ResponseCallback.h"

#include "swcdb/ranger/db/ColumnCfg.h"
#include "swcdb/ranger/db/Callbacks.h"
#include "swcdb/ranger/db/Column.h"

#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC { namespace Ranger {

typedef std::unordered_map<int64_t, Column::Ptr>  ColumnsMap;



class Columns final {

  public:

  enum State{
    OK
  };
  typedef Columns* Ptr;

  Columns() : m_state(State::OK) {}

  virtual ~Columns() { }

  
  Column::Ptr initialize(int &err, const int64_t cid, 
                         const DB::Schema& schema) {
    Column::Ptr col = nullptr;
    if(RangerEnv::is_shuttingdown()) {
      err = Error::SERVER_SHUTTING_DOWN;
      return col;
    }
    
    std::scoped_lock lock(m_mutex);
    auto it = m_columns.find(cid);
    if(it != m_columns.end())
      (col = it->second)->cfg.update(schema);
    else
      m_columns.emplace(cid, col = std::make_shared<Column>(cid, schema));
    return col;
  }

  Column::Ptr get_column(int &err, const int64_t cid) {
    std::shared_lock lock(m_mutex);
    auto it = m_columns.find(cid);
    return it == m_columns.end() ? nullptr : it->second;
  }
  
  Column::Ptr get_next(size_t& idx) {
    std::shared_lock lock(m_mutex);

    if(m_columns.size() > idx){
      auto it = m_columns.begin();
      for(int i=idx; i; --i, ++it);
      return it->second;
    }
    idx = 0;
    return nullptr;
  }

  RangePtr get_range(int &err, const int64_t cid, const int64_t rid) {
    Column::Ptr col = get_column(err, cid);
    if(col == nullptr) 
      return nullptr;
    if(col->removing()) 
      err = Error::COLUMN_MARKED_REMOVED;
    return err ? nullptr : col->get_range(err, rid, false);
  }
 
  void load_range(int &err, const int64_t cid, const int64_t rid, 
                  const DB::Schema& schema, ResponseCallback::Ptr cb) {
    RangePtr range;
    auto col = initialize(err, cid, schema);
    if(!err) {

      if(col->removing()) 
        err = Error::COLUMN_MARKED_REMOVED;

      else if(RangerEnv::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      
      if(!err)
        range = col->get_range(err, rid, true);
    }
    if(err)
      cb->response(err);
    else 
      range->load(cb);
  }

  void unload_range(int &err, const int64_t cid, const int64_t rid,
                    Callback::RangeUnloaded_t cb){
    Column::Ptr col = get_column(err, cid);
    if(col != nullptr) {
      col->unload(rid, cb);
    } else {
      cb(err);
    }
  }

  void unload_all(bool validation) {
    std::vector<std::function<bool(int64_t)>> order = {
      Types::MetaColumn::is_data,
      Types::MetaColumn::is_meta,
      Types::MetaColumn::is_master 
    };

    std::atomic<int> to_unload = 0;
    Column::Ptr col;
    uint8_t meta;
    for(auto chk : order) {

      std::promise<void>  r_promise;
      Callback::RangeUnloaded_t cb 
        = [&to_unload, &r_promise](int err){
          if(--to_unload == 0)
            r_promise.set_value();
      };
    
      for(meta=0;;) {
        {
          std::scoped_lock lock(m_mutex);
          auto it = m_columns.begin();
          for(uint8_t n=0; n<meta; ++n, ++it);
          if(it == m_columns.end())
            break;
          for(;!chk(it->first) && m_columns.size() > ++meta; ++it);
          if(m_columns.size() == meta)
            break;
          col = it->second;
          m_columns.erase(it);
        }
        if(validation)
          SWC_LOGF(LOG_WARN, "Unload-Validation cid=%d remained", col->cfg.cid);
        ++to_unload;
        col->unload_all(to_unload, cb);
      }
      if(to_unload) 
        r_promise.get_future().wait();
    }
  }

  void remove(int &err, const int64_t cid, Callback::ColumnDeleted_t cb) {
    Column::Ptr col = get_column(err, cid);
    if(col != nullptr) {
      col->remove_all(err);
      {
        std::scoped_lock lock(m_mutex);
        auto it = m_columns.find(cid);
        if (it != m_columns.end()) 
          m_columns.erase(it);
      }
    }
    cb(err);
  }
  
  size_t release(size_t bytes=0) {
    size_t released = 0;
    Column::Ptr col;
    ColumnsMap::iterator it;
    for(size_t offset = 0; ; ++offset) {
      {
        std::shared_lock lock(m_mutex);
        it = m_columns.begin();
        for(size_t i=0; i<offset && it != m_columns.end(); ++it, ++i);
        if(it == m_columns.end())
          break;
        if(it->first < 3)
          continue;
        col = it->second;
      }
      released += col->release(bytes ? bytes-released : bytes);
      if(bytes && released >= bytes)
        break;
    }
    return released;
  }

  std::string to_string() {
    std::shared_lock lock(m_mutex);

    std::string s("columns=(");
    for(auto it = m_columns.begin(); it != m_columns.end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")");
    return s;
  }

  private:
  std::shared_mutex m_mutex;
  ColumnsMap        m_columns;
  State             m_state;

};

}} // namespace SWC::Ranger

#endif
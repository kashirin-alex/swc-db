/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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
    if(RangerEnv::is_shuttingdown()) {
      err = Error::SERVER_SHUTTING_DOWN;
      return nullptr;
    }

    Column::Ptr col = nullptr;
    {
      std::scoped_lock lock(m_mutex);
      auto it = m_columns.find(cid);
      if (it != m_columns.end()) {
        col = it->second;
      } else {
        m_columns.emplace(cid, col = std::make_shared<Column>(cid));
      }
      col->cfg.update(schema);
    }
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
      for(int i=idx;i--;++it);
      return it->second;
    }
    idx = 0;
    return nullptr;
  }

  Range::Ptr get_range(int &err, const int64_t cid, const int64_t rid) {
    Column::Ptr col = get_column(err, cid);
    if(col == nullptr) 
      return nullptr;
    if(col->removing()) 
      err = Error::COLUMN_MARKED_REMOVED;
    return err ? nullptr : col->get_range(err, rid, false);
  }
 
  void load_range(int &err, const int64_t cid, const int64_t rid, 
                  const DB::Schema& schema, ResponseCallback::Ptr cb) {
    Range::Ptr range;
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

    std::atomic<int>    unloaded = 0;
    std::promise<void>  r_promise;
    Callback::RangeUnloaded_t cb 
      = [&unloaded, await=&r_promise](int err){
        if(--unloaded == 0)
          await->set_value();
    };

    for(;;){
      std::scoped_lock lock(m_mutex);
      auto it = m_columns.begin();
      if(it == m_columns.end())
        break;
      if(validation)
        SWC_LOGF(LOG_WARN, "Unload-Validation cid=%d remained", it->first);
      ++unloaded;
      it->second->unload_all(unloaded, cb);
      m_columns.erase(it);
    }

    if(unloaded) 
      r_promise.get_future().wait();
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
  
  const size_t release(size_t bytes=0) {
    size_t released = 0;
    Column::Ptr col;
    ColumnsMap::iterator it;
    bool started = false;
    for(;;) {
      {
        std::shared_lock lock(m_mutex);
        if(!started) { 
          it = m_columns.begin();
          started = true;
        } else
          ++it;
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

  const std::string to_string() {
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
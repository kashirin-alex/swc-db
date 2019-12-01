/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Rgr_Columns_Columns_h
#define swcdb_lib_db_Rgr_Columns_Columns_h

#include "swcdb/fs/Interface.h"
#include "swcdb/db/Columns/Schema.h"

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Columns/Rgr/Callbacks.h"
#include "swcdb/db/Columns/Rgr/Column.h"

#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC { namespace server { namespace Rgr {

typedef std::unordered_map<int64_t, Column::Ptr>  ColumnsMap;
typedef std::pair<int64_t, Column::Ptr>           ColumnsMapPair;



class Columns : public std::enable_shared_from_this<Columns> {

  public:

  enum State{
    OK
  };
  typedef Columns* Ptr;

  Columns() : m_state(State::OK) {}

  virtual ~Columns(){}

  Column::Ptr get_column(int &err, const int64_t cid, bool initialize){
    Column::Ptr col = nullptr;
    {
      std::lock_guard lock(m_mutex);

      auto it = m_columns.find(cid);
      if (it != m_columns.end())
        col = it->second;
        
      else if(initialize && !Env::RgrData::is_shuttingdown()) {
        col = std::make_shared<Column>(cid);
        m_columns.insert(ColumnsMapPair(cid, col));
      }
    }

    if(initialize) {
      if(Env::RgrData::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      else
        col->init(err);
    }
    return col;
  }
  
  Column::Ptr get_next(size_t& idx) {
    std::shared_lock lock(m_mutex);

    if(m_columns.size() > idx){
      auto it = m_columns.begin();
      for(int i=idx;i--;it++);
      return it->second;
    }
    idx = 0;
    return nullptr;
  }

  Range::Ptr get_range(int &err, const int64_t cid, const int64_t rid,
                       bool initialize=false) {
    Column::Ptr col = get_column(err, cid, initialize);
    if(col == nullptr) 
      return nullptr;
    if(col->removing()) 
      err = Error::COLUMN_MARKED_REMOVED;
    if(err != Error::OK) 
      return nullptr;

    return col->get_range(err, rid, initialize);
  }

  void load_range(int &err, const int64_t cid, const int64_t rid, 
                  ResponseCallback::Ptr cb){
    Range::Ptr range;
    
    if(Env::RgrData::is_shuttingdown())
      err = Error::SERVER_SHUTTING_DOWN;

    else if(Env::Schemas::get()->get(cid) == nullptr)
      err = Error::COLUMN_SCHEMA_MISSING;

    else {
      range = get_range(err, cid, rid, true);
      if(Env::RgrData::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
    }

    if(err != Error::OK)
      cb->response(err);
    else 
      range->load(cb);
  }

  void unload_range(int &err, const int64_t cid, const int64_t rid,
                    Callback::RangeUnloaded_t cb){
    Column::Ptr col = get_column(err, cid, false);
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
        std::cout << "unloaded= " << unloaded.load() << " err=" << err << "\n";
    };

    for(;;){
      std::lock_guard lock(m_mutex);
      auto it = m_columns.begin();
      if(it == m_columns.end())
        break;
      if(validation)
        SWC_LOGF(LOG_WARN, "Unload-Validation cid=%d remained", it->first);
      unloaded++;
      it->second->unload_all(unloaded, cb);
      m_columns.erase(it);
    }

    if(unloaded > 0) 
      r_promise.get_future().wait();
  }

  void remove(int &err, const int64_t cid, Callback::ColumnDeleted_t cb) {
    Column::Ptr col = get_column(err, cid, false);
    if(col != nullptr) {
      col->remove_all(err);
      {
        std::lock_guard lock(m_mutex);
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
          it++;
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

}} // namespace server::Rgr



namespace Env {
class RgrColumns {
  public:

  static void init() {
    m_env = std::make_shared<RgrColumns>();
  }

  static server::Rgr::Columns::Ptr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_columns;
  }

  RgrColumns() : m_columns(new server::Rgr::Columns()) {}
  virtual ~RgrColumns(){
    if(m_columns != nullptr)
      delete m_columns;
  }

  private:
  server::Rgr::Columns::Ptr                 m_columns = nullptr;
  inline static std::shared_ptr<RgrColumns> m_env = nullptr;
};
}


}
#endif
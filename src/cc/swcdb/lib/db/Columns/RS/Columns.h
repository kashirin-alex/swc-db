/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_RS_Columns_Columns_h
#define swcdb_lib_db_RS_Columns_Columns_h

#include "swcdb/lib/fs/Interface.h"

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "Callbacks.h"
#include "Column.h"

#include <mutex>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC { namespace server { namespace RS {

typedef std::unordered_map<int64_t, ColumnPtr> ColumnsMap;
typedef std::pair<int64_t, ColumnPtr> ColumnsMapPair;



class Columns : public std::enable_shared_from_this<Columns> {

  public:

  Columns() : columns(std::make_shared<ColumnsMap>()) {}

  virtual ~Columns(){}

  ColumnPtr get_column(int64_t cid, bool initialize){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = columns->find(cid);
    if (it != columns->end())
      return it->second;

    if(initialize) {
      ColumnPtr col = std::make_shared<Column>(cid);
      if(col->load())
        columns->insert(ColumnsMapPair(cid, col));
      return col;
    }
    return nullptr;
  }

  RangePtr get_range(int64_t cid, int64_t rid,  bool initialize=false){
    ColumnPtr col = get_column(cid, initialize);
    if(col == nullptr) 
      return nullptr;
    return col->get_range(rid, initialize);
  }

  void load_range(int64_t cid, int64_t rid, ResponseCallbackPtr cb){
    RangePtr range = get_range(cid, rid, true);
    if(range != nullptr && range->is_loaded()) {
      cb->response_ok();
      return;
    }
    
    cb->send_error(Error::NOT_LOADED_RANGE , "");
  }

  void unload_range(int64_t cid, int64_t rid, Callback::RangeUnloaded_t cb){
    ColumnPtr col = get_column(cid, false);
    if(col != nullptr) 
      col->unload(rid);
    
    cb(true);
  }

  void unload_all(){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(;;){
      auto it = columns->begin();
      if(it == columns->end())
        break;
      it->second->unload_all();
      columns->erase(it);
    }
  }
  
  std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("columns=(");
    for(auto it = columns->begin(); it != columns->end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")");
    return s;
  }

  private:
  std::mutex                  m_mutex;
  std::shared_ptr<ColumnsMap> columns;

};
typedef std::shared_ptr<Columns> ColumnsPtr;

}} // namespace server::RS



class EnvRsColumns {
  public:

  static void init() {
    m_env = std::make_shared<EnvRsColumns>();
  }

  static server::RS::ColumnsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_columns;
  }

  EnvRsColumns() : m_columns(std::make_shared<server::RS::Columns>()) {}
  virtual ~EnvRsColumns(){}

  private:
  server::RS::ColumnsPtr                      m_columns = nullptr;
  inline static std::shared_ptr<EnvRsColumns> m_env = nullptr;
};


}
#endif
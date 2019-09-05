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

  Columns() : m_columns(std::make_shared<ColumnsMap>()) {}

  virtual ~Columns(){}

  ColumnPtr get_column(int &err, int64_t cid, bool initialize){
    ColumnPtr col = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      auto it = m_columns->find(cid);
      if (it != m_columns->end())
        col = it->second;
        
      else if(initialize) {
        col = std::make_shared<Column>(cid);
        m_columns->insert(ColumnsMapPair(cid, col));
      }
    }
    if(initialize) 
      col->init(err);
    return col;
  }

  RangePtr get_range(int &err, int64_t cid, int64_t rid,  bool initialize=false){
    ColumnPtr col = get_column(err, cid, initialize);
    if(col == nullptr || col->removing()) 
      return nullptr;
    return col->get_range(err, rid, initialize);
  }

  void load_range(int &err, int64_t cid, int64_t rid, ResponseCallbackPtr cb){
    RangePtr range = get_range(err, cid, rid, true);
    if(err != Error::OK)
      cb->send_error(err , "");

    else if(range != nullptr && range->is_loaded()) 
      cb->response_ok(); // cb->run();
      
    else
      cb->send_error(Error::RS_NOT_LOADED_RANGE , "");
  }

  void unload_range(int &err, int64_t cid, int64_t rid, Callback::RangeUnloaded_t cb){
    ColumnPtr col = get_column(err, cid, false);
    if(col != nullptr) 
      col->unload(err, rid);
    
    cb(err);
  }

  void unload_all(int &err){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(;;){
      auto it = m_columns->begin();
      if(it == m_columns->end())
        break;
      it->second->unload_all(err);
      m_columns->erase(it);
    }
  }

  void remove(int &err, int64_t cid, Callback::ColumnDeleted_t cb){
    ColumnPtr col = get_column(err, cid, false);
    if(col != nullptr) {
      col->remove_all(err);
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_columns->find(cid);
        if (it != m_columns->end()) 
          m_columns->erase(it);
      }
    }
    cb(err);
  }
  
  std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("columns=(");
    for(auto it = m_columns->begin(); it != m_columns->end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")");
    return s;
  }

  private:
  std::mutex                  m_mutex;
  std::shared_ptr<ColumnsMap> m_columns;

};
typedef std::shared_ptr<Columns> ColumnsPtr;

}} // namespace server::RS



namespace Env {
class RsColumns {
  public:

  static void init() {
    m_env = std::make_shared<RsColumns>();
  }

  static server::RS::ColumnsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_columns;
  }

  RsColumns() : m_columns(std::make_shared<server::RS::Columns>()) {}
  virtual ~RsColumns(){}

  private:
  server::RS::ColumnsPtr                   m_columns = nullptr;
  inline static std::shared_ptr<RsColumns> m_env = nullptr;
};
}

}
#endif
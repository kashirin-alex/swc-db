/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Mngr_Columns_Columns_h
#define swcdb_lib_db_Mngr_Columns_Columns_h

#include "swcdb/lib/fs/Interface.h"

#include "Column.h"

#include <mutex>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC { namespace server { namespace Mngr {

typedef std::unordered_map<int64_t, ColumnPtr> ColumnsMap;
typedef std::pair<int64_t, ColumnPtr> ColumnsMapPair;


class Columns : public std::enable_shared_from_this<Columns> {

  public:

  static void columns_by_fs(int &err, FS::IdEntries_t &entries){
    Env::FsInterface::interface()->get_structured_ids(
      err, Range::get_column_path(), entries);
  }

  Columns() : m_columns(std::make_shared<ColumnsMap>()) {}

  void reset(){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_columns = std::make_shared<ColumnsMap>();
  }

  virtual ~Columns(){}

  bool is_an_initialization(int &err, int64_t cid){
    ColumnPtr col = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      auto it = m_columns->find(cid);
      if (it != m_columns->end())
        return false;

      col = std::make_shared<Column>(cid);
      m_columns->insert(ColumnsMapPair(cid, col));
    }

    col->init(err);
    return true;
  }

  ColumnPtr get_column(int &err, int64_t cid, bool initialize){
    ColumnPtr col = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      auto it = m_columns->find(cid);
      if (it != m_columns->end())
        return it->second;
        
      else if(initialize) {
        col = std::make_shared<Column>(cid);
        m_columns->insert(ColumnsMapPair(cid, col));
      }
    }
    if(initialize) 
      col->init(err);
    else
      err = Error::COLUMN_NOT_EXISTS;
    return col;
  }

  Range::Ptr get_range(int &err, int64_t cid, int64_t rid, 
                       bool initialize=false){
    ColumnPtr col = get_column(err, cid, initialize);
    if(col == nullptr) 
      return nullptr;
    return col->get_range(err, rid, initialize);
  }

  Range::Ptr get_next_unassigned(){
    Range::Ptr range = nullptr;
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_columns->begin(); it != m_columns->end(); ++it){
      range = it->second->get_next_unassigned();
      if(range != nullptr)
        break;
    }
    return range;
  }

  void set_rgr_unassigned(uint64_t id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_columns->begin(); it != m_columns->end(); ++it)
      it->second->set_rgr_unassigned(id);
  }

  void change_rgr(uint64_t id_old, uint64_t id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_columns->begin(); it != m_columns->end(); ++it)
      it->second->change_rgr(id_old, id);
  }
  
  int64_t get_next_cid(){
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t cid = 0;
    while(m_columns->find(++cid) != m_columns->end());
    return cid;
  }

  void remove(int &err, int64_t cid){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_columns->find(cid);
    if (it != m_columns->end())
      m_columns->erase(it);
  }

  std::string to_string(){
    std::string s("ColumnsAssignment:");
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto it = m_columns->begin(); it != m_columns->end(); ++it){
      s.append("\n ");
      s.append(it->second->to_string());
    }
    return s;
  }

  private:
  std::mutex                  m_mutex;
  std::shared_ptr<ColumnsMap> m_columns;

};
typedef std::shared_ptr<Columns> ColumnsPtr;

}} // namespace server::Mngr



namespace Env {
class MngrColumns {
  public:

  static void init() {
    m_env = std::make_shared<MngrColumns>();
  }

  static server::Mngr::ColumnsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_columns;
  }

  MngrColumns() : m_columns(std::make_shared<server::Mngr::Columns>()) {}
  virtual ~MngrColumns(){}

  private:
  server::Mngr::ColumnsPtr                   m_columns = nullptr;
  inline static std::shared_ptr<MngrColumns> m_env = nullptr;
};
}

}
#endif
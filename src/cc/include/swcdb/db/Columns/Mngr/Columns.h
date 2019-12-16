/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Mngr_Columns_Columns_h
#define swcdb_lib_db_Mngr_Columns_Columns_h

#include "swcdb/fs/Interface.h"

#include "swcdb/db/Columns/Mngr/ColumnCfg.h"
#include "swcdb/db/Columns/Mngr/Column.h"

#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC { namespace server { namespace Mngr {

typedef std::unordered_map<int64_t, Column::Ptr>  ColumnsMap;
typedef std::pair<int64_t, Column::Ptr>           ColumnsMapPair;


class Columns final {

  public:

  static void columns_by_fs(int &err, FS::IdEntries_t &entries) {
    Env::FsInterface::interface()->get_structured_ids(
      err, Range::get_column_path(), entries);
  }

  typedef Columns* Ptr;

  Columns()  {}

  void reset() {
    std::scoped_lock lock(m_mutex);
    m_columns.clear();
  }

  virtual ~Columns(){}

  bool is_an_initialization(int &err, const int64_t cid) {
    Column::Ptr col = nullptr;
    {
      std::scoped_lock lock(m_mutex);

      auto it = m_columns.find(cid);
      if (it != m_columns.end())
        return false;

      col = std::make_shared<Column>(cid);
      m_columns.insert(ColumnsMapPair(cid, col));
    }

    col->init(err);
    return true;
  }

  Column::Ptr get_column(int &err, const int64_t cid, bool initialize) {
    Column::Ptr col = nullptr;
    {
      std::scoped_lock lock(m_mutex);

      auto it = m_columns.find(cid);
      if (it != m_columns.end())
        return it->second;
        
      else if(initialize) {
        col = std::make_shared<Column>(cid);
        m_columns.insert(ColumnsMapPair(cid, col));
      }
    }
    if(initialize) 
      col->init(err);
    else
      err = Error::COLUMN_NOT_EXISTS;
    return col;
  }

  Range::Ptr get_range(int &err, const int64_t cid, const int64_t rid, 
                       bool initialize=false) {
    Column::Ptr col = get_column(err, cid, initialize);
    if(col == nullptr) 
      return nullptr;
    return col->get_range(err, rid, initialize);
  }

  Range::Ptr get_next_unassigned() {
    Range::Ptr range = nullptr;
    std::shared_lock lock(m_mutex);

    for(auto it = m_columns.begin(); it != m_columns.end(); ++it){
      range = it->second->get_next_unassigned();
      if(range != nullptr)
        break;
    }
    return range;
  }

  void set_rgr_unassigned(uint64_t id) {
    std::shared_lock lock(m_mutex);

    for(auto it = m_columns.begin(); it != m_columns.end(); ++it)
      it->second->set_rgr_unassigned(id);
  }

  void change_rgr(uint64_t id_old, uint64_t id) {
    std::shared_lock lock(m_mutex);

    for(auto it = m_columns.begin(); it != m_columns.end(); ++it)
      it->second->change_rgr(id_old, id);
  }
  
  int64_t get_next_cid(){
    std::shared_lock lock(m_mutex);

    int64_t cid = 0;
    while(m_columns.find(++cid) != m_columns.end());
    return cid;
  }

  void remove(int &err, const int64_t cid) {
    std::scoped_lock lock(m_mutex);

    auto it = m_columns.find(cid);
    if (it != m_columns.end())
      m_columns.erase(it);
  }

  const std::string to_string() {
    std::string s("ColumnsAssignment:");
    
    std::shared_lock lock(m_mutex);
    for(auto it = m_columns.begin(); it != m_columns.end(); ++it){
      s.append("\n ");
      s.append(it->second->to_string());
    }
    return s;
  }

  private:
  std::shared_mutex   m_mutex;
  ColumnsMap          m_columns;

};

}} // namespace server::Mngr



namespace Env {
class MngrColumns final {
  public:

  static void init() {
    m_env = std::make_shared<MngrColumns>();
  }

  static server::Mngr::Columns::Ptr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_columns;
  }

  MngrColumns() : m_columns(new server::Mngr::Columns()) {}

  ~MngrColumns() {
    if(m_columns != nullptr)
      delete m_columns;
  }

  private:
  server::Mngr::Columns::Ptr                 m_columns = nullptr;
  inline static std::shared_ptr<MngrColumns> m_env = nullptr;
};
}

}
#endif
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_MNGR_Columns_Columns_h
#define swcdb_lib_db_MNGR_Columns_Columns_h

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

  Columns() : m_columns(std::make_shared<ColumnsMap>()) {}

  virtual ~Columns(){}

  ColumnPtr get_column(int64_t cid, bool initialize){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_columns->find(cid);
    if (it != m_columns->end())
      return it->second;

    if(initialize) {
      ColumnPtr col = std::make_shared<Column>(cid);
      m_columns->insert(ColumnsMapPair(cid, col));
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

  RangePtr get_next_unassigned(){
    RangePtr range = nullptr;
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_columns->begin(); it != m_columns->end(); ++it){
      range = it->second->get_next_unassigned();
      if(range != nullptr)
        break;
    }
    return range;
  }

  void set_rs_unassigned(uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_columns->begin(); it != m_columns->end(); ++it)
      it->second->set_rs_unassigned(rs_id);
  }

  void change_rs(uint64_t rs_id_old, uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_columns->begin(); it != m_columns->end(); ++it)
      it->second->change_rs(rs_id_old, rs_id);
  }
  
  int64_t get_next_cid(){
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t cid = 0;
    for(;;){
      auto it = m_columns->find(++cid);
      if(it == m_columns->end())
        break;
      if(it->second->deleted())
        break;
    }
    return cid;
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

}} // namespace server::MNGR



class EnvMngrColumns {
  public:

  static void init() {
    m_env = std::make_shared<EnvMngrColumns>();
  }

  static server::Mngr::ColumnsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_columns;
  }

  EnvMngrColumns() : m_columns(std::make_shared<server::Mngr::Columns>()) {}
  virtual ~EnvMngrColumns(){}

  private:
  server::Mngr::ColumnsPtr                      m_columns = nullptr;
  inline static std::shared_ptr<EnvMngrColumns> m_env = nullptr;
};


}
#endif
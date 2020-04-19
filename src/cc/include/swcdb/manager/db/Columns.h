/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_db_Columns_h
#define swc_manager_db_Columns_h

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/fs/Interface.h"

#include "swcdb/manager/db/ColumnCfg.h"
#include "swcdb/manager/db/Column.h"

#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC { namespace Manager {

typedef std::unordered_map<int64_t, Column::Ptr>  ColumnsMap;


class Columns final {

  public:

  static void columns_by_fs(int &err, FS::IdEntries_t &entries) {
    Env::FsInterface::interface()->get_structured_ids(
      err, DB::RangeBase::get_column_path(), entries);
  }

  typedef Columns* Ptr;

  Columns()  {}

  void reset() {
    std::scoped_lock lock(m_mutex);
    m_columns.clear();
  }

  virtual ~Columns(){}

  bool is_an_initialization(int &err, DB::Schema::Ptr schema) {
    Column::Ptr col = nullptr;
    {
      std::scoped_lock lock(m_mutex);

      auto it = m_columns.find(schema->cid);
      if (it != m_columns.end())
        return false;

      m_columns.emplace(schema->cid, col = std::make_shared<Column>(schema));
    }

    col->init(err);
    return true;
  }

  Column::Ptr get_column(int &err, const int64_t cid) {
    
    std::scoped_lock lock(m_mutex);
    auto it = m_columns.find(cid);
    if(it != m_columns.end())
      return it->second;
    err = Error::COLUMN_NOT_EXISTS;
    return nullptr;
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

  std::string to_string() {
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

}} // namespace SWC::Manager

#endif // swc_manager_db_Columns_h
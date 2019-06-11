/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_rs_Columns_h
#define swcdb_lib_rs_Columns_h

#include "Column.h"

#include <mutex>
#include <memory>
#include <unordered_map>


namespace SWC {

typedef std::unordered_map<int64_t, ColumnPtr> ColumnsMap;
typedef std::pair<int64_t, ColumnPtr> ColumnsMapPair;



class Columns : public std::enable_shared_from_this<Columns> {

  public:

  Columns(){
    columns = std::make_shared<ColumnsMap>();
  }
  virtual ~Columns(){}
  
  bool load_range(int64_t cid, int64_t rid){
    return get_range(cid, rid, true) != nullptr;
  }

  RangePtr get_range(int64_t cid, int64_t rid, bool load=false){
    ColumnPtr col = get_column(cid, load);
    if(col == nullptr) 
      return nullptr;
    return col->get_range(rid, load);
  }

  private:

  ColumnPtr get_column(int64_t cid, bool load){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = columns->find(cid);
    if (it != columns->end())
      return it->second;

    if(load) {
      ColumnPtr col = std::make_shared<Column>(cid);
      columns->insert(ColumnsMapPair(cid, col));
      return col;
    }
    return nullptr;
  }

  std::mutex m_mutex;
  std::shared_ptr<ColumnsMap> columns;

};

typedef std::shared_ptr<Columns> ColumnsPtr;


}
#endif
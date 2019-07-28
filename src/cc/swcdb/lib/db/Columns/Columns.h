/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Columns_h
#define swcdb_lib_db_Columns_Columns_h

#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/db/Types/RsRole.h"

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "Column.h"

#include <mutex>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace SWC {

typedef std::unordered_map<int64_t, ColumnPtr> ColumnsMap;
typedef std::pair<int64_t, ColumnPtr> ColumnsMapPair;



class Columns : public std::enable_shared_from_this<Columns> {

  public:

  Columns(FS::InterfacePtr fs) 
          : m_fs(fs),
            columns(std::make_shared<ColumnsMap>()) {
  }


  virtual ~Columns(){}

  ColumnPtr get_column(int64_t cid, bool load){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = columns->find(cid);
    if (it != columns->end())
      return it->second;

    if(load) {
      ColumnPtr col = std::make_shared<Column>(m_fs, cid);
      if(col->has_err() == 0)
        columns->insert(ColumnsMapPair(cid, col));
      return col;
    }
    return nullptr;
  }

  RangePtr get_range(int64_t cid, int64_t rid, bool initialize=false, bool load=false){
    ColumnPtr col = get_column(cid, load);
    if(col == nullptr) 
      return nullptr;
    return col->get_range(rid, initialize, load);
  }

  void load_range(int64_t cid, int64_t rid, ResponseCallbackPtr cb){
    RangePtr range = get_range(cid, rid, true, true);
    if(range != nullptr && range->is_loaded()) {
      cb->response_ok();
      return;
    }
    cb->response_ok();
  }

  private:
  std::mutex                  m_mutex;
  std::shared_ptr<ColumnsMap> columns;
  FS::InterfacePtr            m_fs;

};

typedef std::shared_ptr<Columns> ColumnsPtr;


}
#endif
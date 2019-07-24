/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_rs_Columns_h
#define swcdb_lib_rs_Columns_h

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
  
  void load_master_ranges(ResponseCallbackPtr cb){
    
    // list master columns >> ranges


    cb->response_ok();
    std::cout << "  load_master_ranges \n";
  }

  void load_range(Types::RsRole role, int64_t cid, int64_t rid,
                  ResponseCallbackPtr cb){
    bool loaded = get_range(cid, rid, true) != nullptr;
    cb->response_ok();
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
  FS::InterfacePtr m_fs;
};

typedef std::shared_ptr<Columns> ColumnsPtr;


}
#endif
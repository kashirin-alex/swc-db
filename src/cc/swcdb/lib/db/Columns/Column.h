/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Column_h
#define swcdb_lib_db_Columns_Column_h

#include "Schema.h"
#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>

namespace SWC { namespace DB {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  Column(int64_t id) : cid(id), ranges(std::make_shared<RangesMap>()) { }

  bool load() {
    std::string col_range_path(Range::get_path(cid));
    int err = Error::OK;
    if(!EnvFsInterface::fs()->exists(err, col_range_path)) {
      EnvFsInterface::fs()->mkdirs(err, col_range_path);
      if(err == 17)
        err = Error::OK;
    }
    return err == Error::OK;
  }

  virtual ~Column(){}

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    EnvFsInterface::interface()->get_structured_ids(err, Range::get_path(cid), entries);
  }

  RangePtr get_range(int64_t rid, bool initialize=false, bool is_rs=false){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = ranges->find(rid);
    if (it != ranges->end())
      return it->second;

    if(initialize) {
      RangePtr range = std::make_shared<Range>(cid, rid, m_schema);
      if(!is_rs || range->load()) {
        ranges->insert(RangesMapPair(rid, range));
      }
      return range;
    }
    return nullptr;
  }

  void unload(int64_t rid){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = ranges->find(rid);
    if (it != ranges->end()){
      it->second->unload();
      ranges->erase(it);
    }
  }

  void unload_all(){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(;;){
      auto it = ranges->begin();
      if(it == ranges->end())
        break;
      it->second->unload();
      ranges->erase(it);
    }
  }

  private:

  std::mutex        m_mutex;
  
  int64_t           cid;
  SchemaPtr         m_schema;
  std::shared_ptr<RangesMap> ranges;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}

#endif
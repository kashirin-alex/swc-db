/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_rs_Column_h
#define swcdb_lib_rs_Column_h

#include "Schema.h"
#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>


namespace SWC {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:
  Column(int64_t id): cid(id){
    //get_schema
    ranges = std::make_shared<RangesMap>();
  }
  virtual ~Column(){}

  RangePtr get_range(int64_t rid, bool load=false){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = ranges->find(rid);
    if (it != ranges->end())
      return it->second;

    if(load) {
      RangePtr range = std::make_shared<Range>(cid, schema, rid);
      ranges->insert(RangesMapPair(rid, range));
      return range;
    }
    return nullptr;
  }

  private:

  std::mutex  m_mutex;
  int64_t     cid;
  SchemaPtr   schema;
  std::shared_ptr<RangesMap> ranges;

};
typedef std::shared_ptr<Column> ColumnPtr;

}

#endif
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Column_h
#define swcdb_lib_db_Columns_RS_Column_h

#include "swcdb/lib/db/Columns/Schema.h"
#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>

namespace SWC { namespace server { namespace RS {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  Column(int64_t id) 
        : cid(id), m_ranges(std::make_shared<RangesMap>()) { }

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

  RangePtr get_range(int64_t rid, bool initialize=false){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_ranges->find(rid);
    if (it != m_ranges->end())
      return it->second;

    if(initialize) {
      RangePtr range = std::make_shared<Range>(cid, rid, m_schema);
      if(range->load()) 
        m_ranges->insert(RangesMapPair(rid, range));
      return range;
    }
    return nullptr;
  }

  void unload(int64_t rid){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_ranges->find(rid);
    if (it != m_ranges->end()){
      it->second->unload();
      m_ranges->erase(it);
    }
  }

  void unload_all(){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(;;){
      auto it = m_ranges->begin();
      if(it == m_ranges->end())
        break;
      it->second->unload();
      m_ranges->erase(it);
    }
  }
  
  std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("[cid=");
    s.append(std::to_string(cid));
    s.append(", ranges=(");
    
    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")]");
    return s;
  }

  private:

  std::mutex                 m_mutex;
  int64_t                    cid;
  SchemaPtr                  m_schema;
  std::shared_ptr<RangesMap> m_ranges;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif
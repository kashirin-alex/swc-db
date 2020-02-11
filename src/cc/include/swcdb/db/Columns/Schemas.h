/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Schemas_h
#define swcdb_lib_db_Columns_Schemas_h


#include "swcdb/db/Columns/Schema.h"

namespace SWC { namespace DB {

class Schemas final {
  public:

  Schemas() {}

  ~Schemas() {}
  
  void add(int &err, Schema::Ptr schema){
    std::scoped_lock lock(m_mutex);
    if(!m_map.emplace(schema->cid, schema).second) {
      SWC_LOGF(LOG_WARN, "Unable to add column %s, remove first", 
                schema->to_string().c_str());
      err = Error::COLUMN_SCHEMA_NAME_EXISTS;
    }
  }

  void remove(int64_t cid){
    std::scoped_lock lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end())
      m_map.erase(it);
  }

  void replace(Schema::Ptr schema){
    std::scoped_lock lock(m_mutex);

    auto it = m_map.find(schema->cid);
    if(it == m_map.end())
       m_map.emplace(schema->cid, schema);
    else
      it->second = schema;
  }

  Schema::Ptr get(int64_t cid){
    std::shared_lock lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      return nullptr;
    return it->second;
  }
  
  Schema::Ptr get(const std::string &name){
    std::shared_lock lock(m_mutex);
    
    for( const auto& it : m_map ) {
      if(name.compare(it.second->col_name) == 0)
        return it.second;
    }
    return nullptr;
  }

  void all(std::vector<Schema::Ptr> &entries){
    size_t i = entries.size();
    std::shared_lock lock(m_mutex);
    entries.resize(i+m_map.size());
    for(const auto& it : m_map) 
      entries[i++] = it.second;
    sort(entries.begin(), entries.end()); 
  }

  private:
  std::shared_mutex                         m_mutex;
  std::unordered_map<int64_t, Schema::Ptr>  m_map;
};



}}
#endif
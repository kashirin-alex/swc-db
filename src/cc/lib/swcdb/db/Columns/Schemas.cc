/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/Error.h"
#include "swcdb/db/Columns/Schemas.h"
#include <algorithm>


namespace SWC { namespace DB {


Schemas::Schemas() {}

Schemas::~Schemas() {}

void Schemas::add(int& err, const Schema::Ptr& schema) {
  std::scoped_lock lock(m_mutex);
  if(!emplace(schema->cid, schema).second) {
    SWC_LOGF(LOG_WARN, "Unable to add column %s, remove first", 
              schema->to_string().c_str());
    err = Error::COLUMN_SCHEMA_NAME_EXISTS;
  }
}

void Schemas::remove(cid_t cid) {
  std::scoped_lock lock(m_mutex);

  auto it = find(cid);
  if(it != end())
    erase(it);
}

void Schemas::replace(const Schema::Ptr& schema) {
  std::scoped_lock lock(m_mutex);

  auto it = find(schema->cid);
  if(it == end())
     emplace(schema->cid, schema);
  else
    it->second = schema;
}

Schema::Ptr Schemas::get(cid_t cid) {
  std::shared_lock lock(m_mutex);

  auto it = find(cid);
  if(it == end())
    return nullptr;
  return it->second;
}

Schema::Ptr Schemas::get(const std::string& name) {
  std::shared_lock lock(m_mutex);
  
  for(const auto& it : *this ) {
    if(name.compare(it.second->col_name) == 0)
      return it.second;
  }
  return nullptr;
}

void Schemas::all(std::vector<Schema::Ptr>& entries) {
  size_t i = entries.size();
  std::shared_lock lock(m_mutex);
  entries.resize(i + size());
  for(const auto& it : *this) 
    entries[i++] = it.second;
  std::sort(entries.begin(), entries.end()); 
}

void Schemas::reset() {
  std::scoped_lock lock(m_mutex);
  clear();
}

}}

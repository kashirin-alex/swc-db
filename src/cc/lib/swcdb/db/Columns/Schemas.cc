/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Error.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace DB {


Schemas::Schemas() {}

Schemas::~Schemas() {}

void Schemas::add(int& err, const Schema::Ptr& schema) {
  Mutex::scope lock(m_mutex);
  _add(err, schema);
}

void Schemas::_add(int& err, const Schema::Ptr& schema) {
  if(!emplace(schema->cid, schema).second) {
    SWC_LOGF(LOG_WARN, "Unable to add column %s, remove first", 
             schema->to_string().c_str());
    err = Error::COLUMN_SCHEMA_NAME_EXISTS;
  }
}

void Schemas::remove(cid_t cid) {
  Mutex::scope lock(m_mutex);
  _remove(cid);
}

void Schemas::_remove(cid_t cid) {
  auto it = find(cid);
  if(it != end())
    erase(it);
}

void Schemas::replace(const Schema::Ptr& schema) {
  Mutex::scope lock(m_mutex);
  _replace(schema);
}

void Schemas::_replace(const Schema::Ptr& schema) {
  auto it = find(schema->cid);
  if(it == end())
     emplace(schema->cid, schema);
  else
    it->second = schema;
}

Schema::Ptr Schemas::get(cid_t cid) {
  Mutex::scope lock(m_mutex);
  return _get(cid);
}

Schema::Ptr Schemas::_get(cid_t cid) const {
  auto it = find(cid);
  return it == end() ? nullptr : it->second;
}

Schema::Ptr Schemas::get(const std::string& name) {
  Mutex::scope lock(m_mutex);
  return _get(name);
}

Schema::Ptr Schemas::_get(const std::string& name) const {
  for(const auto& it : *this ) {
    if(name.compare(it.second->col_name) == 0)
      return it.second;
  }
  return nullptr;
}

void Schemas::all(std::vector<Schema::Ptr>& entries) {
  size_t i = entries.size();
  Mutex::scope lock(m_mutex);
  entries.resize(i + size());
  for(const auto& it : *this) 
    entries[i++] = it.second;
}

void Schemas::matching(const std::vector<Schemas::Pattern>& patterns,
                       std::vector<Schema::Ptr>& entries, bool no_sys) {
  Mutex::scope lock(m_mutex);
  for(const auto& it : *this) {
    if(no_sys && 
       (!Types::MetaColumn::is_data(it.second->cid) || it.second->cid == 9))
      continue;
    for(auto& pattern : patterns) {
      if(Condition::is_matching_extended(
          pattern.comp,
          (const uint8_t*)pattern.value.data(), pattern.value.size(),
          (const uint8_t*)it.second->col_name.data(), 
          it.second->col_name.size() )
        ) {
        entries.push_back(it.second);
        break;
      }
    }
  }
}

void Schemas::reset() {
  Mutex::scope lock(m_mutex);
  clear();
}

}}

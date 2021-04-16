/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace DB {


Schemas::Schemas() noexcept {}

Schemas::~Schemas() {}

void Schemas::add(int& err, const Schema::Ptr& schema) {
  Core::MutexSptd::scope lock(m_mutex);
  _add(err, schema);
}

void Schemas::_add(int& err, const Schema::Ptr& schema) {
  if(!emplace(schema->cid, schema).second) {
    err = Error::COLUMN_SCHEMA_NAME_EXISTS;
    SWC_LOG_OUT(LOG_WARN,
      schema->print(SWC_LOG_OSTREAM << "Unable to add column ");
      Error::print(SWC_LOG_OSTREAM << ", remove first ", err);
    );
  }
}

void Schemas::remove(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  _remove(cid);
}

void Schemas::_remove(cid_t cid) {
  auto it = find(cid);
  if(it != end())
    erase(it);
}

void Schemas::replace(const Schema::Ptr& schema) {
  Core::MutexSptd::scope lock(m_mutex);
  _replace(schema);
}

void Schemas::_replace(const Schema::Ptr& schema) {
  auto res = emplace(schema->cid, schema);
  if(!res.second)
    res.first->second = schema;
}

Schema::Ptr Schemas::get(cid_t cid) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return _get(cid);
}

Schema::Ptr Schemas::_get(cid_t cid) const noexcept {
  auto it = find(cid);
  return it == end() ? nullptr : it->second;
}

Schema::Ptr Schemas::get(const std::string& name) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return _get(name);
}

Schema::Ptr Schemas::_get(const std::string& name) const noexcept {
  for(const auto& it : *this ) {
    if(Condition::str_eq(name, it.second->col_name))
      return it.second;
  }
  return nullptr;
}

void Schemas::all(std::vector<Schema::Ptr>& entries) {
  size_t i = entries.size();
  Core::MutexSptd::scope lock(m_mutex);
  entries.resize(i + size());
  for(const auto& it : *this)
    entries[i++] = it.second;
}

void Schemas::matching(const std::vector<Schemas::Pattern>& patterns,
                       std::vector<Schema::Ptr>& entries, bool no_sys) {
  Core::MutexSptd::scope lock(m_mutex);
  for(const auto& it : *this) {
    if(no_sys &&
       (!Types::MetaColumn::is_data(it.second->cid) || it.second->cid == 9))
      continue;
    for(auto& pattern : patterns) {
      if(Condition::is_matching_extended(
          pattern.comp,
          reinterpret_cast<const uint8_t*>(pattern.value.c_str()),
          pattern.value.size(),
          reinterpret_cast<const uint8_t*>(it.second->col_name.c_str()),
          it.second->col_name.size() )
        ) {
        entries.push_back(it.second);
        break;
      }
    }
  }
}

void Schemas::reset() {
  Core::MutexSptd::scope lock(m_mutex);
  clear();
}

}}

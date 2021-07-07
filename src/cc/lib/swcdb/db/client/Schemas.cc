/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Sync.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnList_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnGet_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnList_Sync.h"

namespace SWC { namespace client {


void Schemas::remove(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);

  auto it = m_track.find(cid);
  if(it != m_track.cend())
    m_track.erase(it);
  _remove(cid);
}

void Schemas::remove(const std::string& name) {
  Core::MutexSptd::scope lock(m_mutex);

  auto schema = _get(name);
  if(!schema)
    return;
  auto it = m_track.find(schema->cid);
  if(it != m_track.cend())
    m_track.erase(it);
  _remove(schema->cid);
}

DB::Schema::Ptr Schemas::get(int& err, cid_t cid) {
  DB::Schema::Ptr schema;
  Core::MutexSptd::scope lock(m_mutex);

  auto it = m_track.find(cid);
  if(it != m_track.cend() &&
     Time::now_ms() - it->second < m_expiry_ms->get() &&
     (schema = _get(cid)))
    return schema;

  _request(err, cid, schema);
  if(schema) {
    m_track.emplace(cid, Time::now_ms());
    _replace(schema);
  } else if(!err)
    err = Error::COLUMN_SCHEMA_MISSING;
  return schema;
}

DB::Schema::Ptr Schemas::get(int& err, const std::string& name) {
  DB::Schema::Ptr schema;
  Core::MutexSptd::scope lock(m_mutex);

  if((schema = _get(name))) {
    auto it = m_track.find(schema->cid);
    if(it != m_track.cend() && Time::now_ms()-it->second < m_expiry_ms->get())
      return schema;
    schema = nullptr;
  }

  _request(err, name, schema);
  if(schema) {
    m_track.emplace(schema->cid, Time::now_ms());
    _replace(schema);
  } else if(!err) {
    err = Error::COLUMN_SCHEMA_MISSING;
  }
  return schema;
}

DB::Schema::Ptr Schemas::get(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = m_track.find(cid);
  return it != m_track.cend() &&
         Time::now_ms()-it->second < m_expiry_ms->get()
         ? _get(cid) : nullptr;
}

DB::Schema::Ptr Schemas::get(const std::string& name) {
  DB::Schema::Ptr schema;
  Core::MutexSptd::scope lock(m_mutex);
  if((schema = _get(name))) {
    auto it = m_track.find(schema->cid);
    if(it == m_track.cend() || Time::now_ms()-it->second > m_expiry_ms->get())
      schema = nullptr;
  }
  return schema;
}

void
Schemas::get(int& err, const std::vector<DB::Schemas::Pattern>& patterns,
             std::vector<DB::Schema::Ptr>& schemas) {
  _request(err, patterns, schemas);

  if(!err && schemas.empty()) {
    err = Error::COLUMN_SCHEMA_MISSING;

  } else if(!err) {
    auto ts = Time::now_ms();
    Core::MutexSptd::scope lock(m_mutex);
    for(auto& schema : schemas) {
      m_track[schema->cid] = ts;
      _replace(schema);
    }
  }
}

std::vector<DB::Schema::Ptr>
Schemas::get(int& err, const std::vector<DB::Schemas::Pattern>& patterns) {
  std::vector<DB::Schema::Ptr> schemas;
  get(err, patterns, schemas);
  return schemas;
}

void Schemas::set(const DB::Schema::Ptr& schema) {
  auto ts = Time::now_ms();
  Core::MutexSptd::scope lock(m_mutex);
  m_track[schema->cid] = ts;
  _replace(schema);
}

void Schemas::set(const std::vector<DB::Schema::Ptr>& schemas) {
  auto ts = Time::now_ms();
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& schema : schemas) {
    m_track[schema->cid] = ts;
    _replace(schema);
  }
}

void Schemas::_request(int& err, cid_t cid,
                       DB::Schema::Ptr& schema) {
  switch(_clients->flags) {
    case Clients::Flag::DEFAULT |Clients::Flag::BROKER |Clients::Flag::SCHEMA:
    case Clients::Flag::BROKER:
      return Comm::Protocol::Bkr::Req::ColumnGet_Sync::schema(
        cid, 300000, _clients->shared(), err, schema);
    default:
      return Comm::Protocol::Mngr::Req::ColumnGet_Sync::schema(
        cid, 300000, _clients->shared(), err, schema);
  }
}

void Schemas::_request(int& err, const std::string& name,
                       DB::Schema::Ptr& schema) {
  switch(_clients->flags) {
    case Clients::Flag::DEFAULT |Clients::Flag::BROKER |Clients::Flag::SCHEMA:
    case Clients::Flag::BROKER:
      return Comm::Protocol::Bkr::Req::ColumnGet_Sync::schema(
        name, 300000, _clients->shared(), err, schema);
    default:
      return Comm::Protocol::Mngr::Req::ColumnGet_Sync::schema(
        name, 300000, _clients->shared(), err, schema);
  }
}

void Schemas::_request(int& err,
                       const std::vector<DB::Schemas::Pattern>& patterns,
                       std::vector<DB::Schema::Ptr>& schemas) {
  Comm::Protocol::Mngr::Params::ColumnListReq params;
  params.patterns = patterns;
  switch(_clients->flags) {
    case Clients::Flag::DEFAULT |Clients::Flag::BROKER |Clients::Flag::SCHEMA:
    case Clients::Flag::BROKER:
      return Comm::Protocol::Bkr::Req::ColumnList_Sync::request(
        params, 300000, _clients->shared(), err, schemas);
    default:
      return Comm::Protocol::Mngr::Req::ColumnList_Sync::request(
        params, 300000, _clients->shared(), err, schemas);
  }
}


}}

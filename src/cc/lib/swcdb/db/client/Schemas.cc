/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"

namespace SWC { namespace client {

Schemas::Schemas(const Property::V_GINT32::Ptr expiry_ms) 
                : m_expiry_ms(expiry_ms) {
} 

Schemas::~Schemas() { }
  
void Schemas::remove(cid_t cid) {    
  Mutex::scope lock(m_mutex);

  auto it = m_track.find(cid);
  if(it != m_track.end()) 
    m_track.erase(it);
  _remove(cid);
}

void Schemas::remove(const std::string& name) {
  Mutex::scope lock(m_mutex);

  auto schema = _get(name);
  if(!schema)
    return;
  auto it = m_track.find(schema->cid);
  if(it != m_track.end()) 
    m_track.erase(it);
  _remove(schema->cid);
}

DB::Schema::Ptr Schemas::get(int& err, cid_t cid) {
  DB::Schema::Ptr schema;
  Mutex::scope lock(m_mutex);

  auto it = m_track.find(cid);
  if(it != m_track.end() && 
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
  Mutex::scope lock(m_mutex);

  if((schema = _get(name))) {
    auto it = m_track.find(schema->cid);
    if(it != m_track.end() && Time::now_ms() - it->second < m_expiry_ms->get())
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

std::vector<DB::Schema::Ptr> 
Schemas::get(int& err, const std::vector<DB::Schemas::Pattern>& patterns) {
  std::vector<DB::Schema::Ptr> schemas;
  _request(err, patterns, schemas);

  if(!err && schemas.empty()) {
    err = Error::COLUMN_SCHEMA_MISSING;

  } else if(!err) {
    Mutex::scope lock(m_mutex);
    for(auto& schema : schemas) {
      m_track[schema->cid] = Time::now_ms();
      _replace(schema);
    }
  }
  return schemas;
}


void Schemas::_request(int& err, cid_t cid, 
                       DB::Schema::Ptr& schema) {
  std::promise<int> res;

  Protocol::Mngr::Req::ColumnGet::schema(
    cid, 
    [schema=&schema, await=&res] 
    (const client::ConnQueue::ReqBase::Ptr& req,
      int error, const Protocol::Mngr::Params::ColumnGetRsp& rsp) {
      if(error == Error::REQUEST_TIMEOUT) {
        SWC_PRINT << " error=" << error 
                  << "(" << Error::get_text(error) << ")" << SWC_PRINT_CLOSE;
        req->request_again();
        return;
      }
      if(!error)
        *schema = rsp.schema;
      await->set_value(error);
    },
    300000
  );

  err = res.get_future().get();
}

void Schemas::_request(int& err, const std::string& name, 
                       DB::Schema::Ptr& schema) {
  std::promise<int> res;

  Protocol::Mngr::Req::ColumnGet::schema(
    name, 
    [schema=&schema, await=&res] 
    (const client::ConnQueue::ReqBase::Ptr& req,
      int error, const Protocol::Mngr::Params::ColumnGetRsp& rsp) {
      if(error == Error::REQUEST_TIMEOUT) {
        SWC_PRINT << " error=" << error 
                  << "(" << Error::get_text(error) << ")" << SWC_PRINT_CLOSE;
        req->request_again();
        return;
      }
      if(!error)
        *schema = rsp.schema;
      await->set_value(error);
    },
    300000
  );

  err = res.get_future().get();
}

void Schemas::_request(int& err, 
                       const std::vector<DB::Schemas::Pattern>& patterns,
                       std::vector<DB::Schema::Ptr>& schemas) {
  Protocol::Mngr::Params::ColumnListReq params;
  params.patterns = patterns;
    
  std::promise<int> res;
  Protocol::Mngr::Req::ColumnList::request(
    params,
    [&schemas, await=&res]
    (const client::ConnQueue::ReqBase::Ptr& req, int error, 
     const Protocol::Mngr::Params::ColumnListRsp& rsp) {
      if(error == Error::REQUEST_TIMEOUT) {
        SWC_PRINT << " error=" << error 
                  << "(" << Error::get_text(error) << ")" << SWC_PRINT_CLOSE;
        req->request_again();
        return;
      }
      if(!error)
        schemas = rsp.schemas;
      await->set_value(error);
    },
    300000
  );
  err = res.get_future().get();
}


}}

/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"

namespace SWC { namespace client {

Schemas::Schemas(const Property::V_GINT32::Ptr expiry_ms) 
                : m_expiry_ms(expiry_ms),
                  m_schemas(std::make_shared<DB::Schemas>()) { }

Schemas::~Schemas() { }
  
void Schemas::remove(int64_t cid){    
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_track.find(cid);
  if(it == m_track.end()) 
    return; 
  m_track.erase(it);
  m_schemas->remove(cid);
}

void Schemas::remove(const std::string &name){
  std::lock_guard<std::mutex> lock(m_mutex);

  auto schema = m_schemas->get(name);
  if(schema == nullptr)
    return;
  auto it = m_track.find(schema->cid);
  if(it != m_track.end()) 
    m_track.erase(it);
  m_schemas->remove(schema->cid);
}

DB::Schema::Ptr Schemas::get(int& err, int64_t cid){
  std::lock_guard<std::mutex> lock(m_mutex);

  DB::Schema::Ptr schema;
  auto it = m_track.find(cid);
  if(it == m_track.end() || Time::now_ms() - it->second > m_expiry_ms->get()) {
    request(err, cid);
    schema = m_schemas->get(cid);
    if(schema != nullptr)
      m_track.emplace(cid, Time::now_ms());
  } else 
    schema = m_schemas->get(cid);

  if(schema == nullptr)
    err = Error::COLUMN_SCHEMA_MISSING;
  return schema;
}
  
DB::Schema::Ptr Schemas::get(int& err, const std::string &name){
  DB::Schema::Ptr schema = m_schemas->get(name);    
  if(schema != nullptr)
    return get(err, schema->cid);
  request(err, name);
  schema = m_schemas->get(name);
  if(schema == nullptr)
    err = Error::COLUMN_SCHEMA_MISSING;
  else {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_track.emplace(schema->cid, Time::now_ms());
  }
  return schema;
}

void Schemas::request(int& err, int64_t cid) {
  std::promise<int> res;

  Protocol::Mngr::Req::ColumnGet::schema(
    cid, 
    [await=&res, schemas=m_schemas] 
    (client::ConnQueue::ReqBase::Ptr req_ptr,
      int error, Protocol::Mngr::Params::ColumnGetRsp rsp) {
      if(error == Error::REQUEST_TIMEOUT) {
        std::cout << " error=" << error << "(" << Error::get_text(error) << ") \n";
        req_ptr->request_again();
        return;
      }
      if(!error)
        schemas->replace(rsp.schema);
      await->set_value(error);
    },
    300000
  );

  err = res.get_future().get();
}

void Schemas::request(int& err, const std::string &name) {
  std::promise<int> res;

  Protocol::Mngr::Req::ColumnGet::schema(
    name, 
    [await=&res, schemas=m_schemas] 
    (client::ConnQueue::ReqBase::Ptr req_ptr,
      int error, Protocol::Mngr::Params::ColumnGetRsp rsp) {
      if(error == Error::REQUEST_TIMEOUT) {
        std::cout << " error=" << error << "(" << Error::get_text(error) << ") \n";
        req_ptr->request_again();
        return;
      }
      if(!error)
        schemas->replace(rsp.schema);
      await->set_value(error);
    },
    300000
  );

  err = res.get_future().get();
}



}}

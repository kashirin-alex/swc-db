/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_Schemas_h
#define swc_client_Schemas_h

#include "swcdb/lib/core/Time.h"
#include "swcdb/lib/db/Columns/Schema.h"
#include "swcdb/lib/db/Protocol/Mngr/req/ColumnGet.h"

namespace SWC { namespace client {

class Schemas  {
  public:

  Schemas(const gInt32tPtr expiry_ms) 
          : m_expiry_ms(expiry_ms),
            m_schemas(std::make_shared<DB::Schemas>()) { }

  virtual ~Schemas(){ }
  
  DB::Schema::Ptr get(int& err, int64_t cid){
    std::lock_guard<std::mutex> lock(m_mutex);

    DB::Schema::Ptr schema;
    auto it = m_track.find(cid);
    if(it == m_track.end() || Time::now_ns() - it->second > m_expiry_ms->get()) {
      request(err, cid);
      schema = m_schemas->get(cid);
      if(schema != nullptr)
        m_track.insert(std::make_pair(cid, Time::now_ns()));
    } else 
      schema = m_schemas->get(cid);

    if(schema == nullptr)
      err = Error::COLUMN_SCHEMA_MISSING;
    return schema;
  }
  
  DB::Schema::Ptr get(int& err, const std::string &name){
    DB::Schema::Ptr schema = m_schemas->get(name);
    if(schema != nullptr)
      return get(err, schema->cid);

    request(err, name);
    schema = m_schemas->get(name);
    if(schema == nullptr)
      err = Error::COLUMN_SCHEMA_MISSING;
    else {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_track.insert(std::make_pair(schema->cid, Time::now_ns()));
    }
    return schema;
  }

  void request(int& err, int64_t cid) {
    std::promise<int> res;

    Protocol::Mngr::Req::ColumnGet::schema(
      cid, 
      [await=&res, schemas=m_schemas] 
      (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req_ptr,
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

  void request(int& err, const std::string &name) {
    std::promise<int> res;

    Protocol::Mngr::Req::ColumnGet::schema(
      name, 
      [await=&res, schemas=m_schemas] 
      (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req_ptr,
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

  private:
  std::mutex                            m_mutex;
  std::unordered_map<int64_t, uint64_t> m_track; // .second {time,queue(promises)}
  std::shared_ptr<DB::Schemas>          m_schemas = nullptr;
  gInt32tPtr                            m_expiry_ms;
  
};



}}

#endif // swc_client_Schemas_h
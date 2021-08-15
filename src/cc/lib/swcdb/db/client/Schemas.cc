/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnList_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnList_Sync.h"
#include "swcdb/core/StateSynchronization.h"

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



class Schemas::ColumnGetData final {
  public:

  SWC_CAN_INLINE
  ColumnGetData(const SWC::client::Clients::Ptr& clients) noexcept
                :  clients(clients), err(Error::OK), once(false) {
  }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return clients;
  }

  SWC_CAN_INLINE
  bool valid() {
    return true;
  }

  SWC_CAN_INLINE
  void callback(const Comm::client::ConnQueue::ReqBase::Ptr&,
                int error,
                const Comm::Protocol::Mngr::Params::ColumnGetRsp& rsp) {
    schema = std::move(rsp.schema);
    err = error;
    await.acknowledge();
  }

  SWC_CAN_INLINE
  bool wait(int& _err, DB::Schema::Ptr& _schema) {
    await.wait();
    _schema = schema;
    _err = err;
    bool at = false;
    return once.compare_exchange_weak(at, true);
  }

  private:
  SWC::client::Clients::Ptr   clients;
  Core::StateSynchronization  await;
  DB::Schema::Ptr             schema;
  int                         err;
  Core::AtomicBool            once;

};



DB::Schema::Ptr
Schemas::get(int& err, cid_t cid, uint32_t timeout) {
  DB::Schema::Ptr schema;
  Pending pending;
  bool has_req;
  {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = m_track.find(cid);
    if(it != m_track.cend() &&
       Time::now_ms() - it->second < m_expiry_ms->get() &&
       (schema = _get(cid))) {
      return schema;
    }
    auto it_req = m_pending_cid.find(cid);
    has_req = it_req != m_pending_cid.cend();
    pending = has_req
      ? it_req->second
      : m_pending_cid.emplace(cid, _request(cid, timeout)).first->second;
  }
  if(!has_req)
    pending.req->run();

  if(pending.datap->wait(err, schema) && schema) {
    auto ts = Time::now_ms();
    Core::MutexSptd::scope lock(m_mutex);
    m_track.emplace(schema->cid, ts);
    _replace(schema);
    m_pending_cid.erase(cid);
  } else if(!schema && !err) {
    err = Error::COLUMN_SCHEMA_MISSING;
  }
  return schema;
}

DB::Schema::Ptr
Schemas::get(int& err, const std::string& name, uint32_t timeout) {
  DB::Schema::Ptr schema;
  Pending pending;
  bool has_req;
  {
    Core::MutexSptd::scope lock(m_mutex);
    if((schema = _get(name))) {
      auto it = m_track.find(schema->cid);
      if(it != m_track.cend() && Time::now_ms()-it->second < m_expiry_ms->get())
        return schema;
    }
    auto it_req = m_pending_name.find(name);
    has_req = it_req != m_pending_name.cend();
    pending = has_req
      ? it_req->second
      : m_pending_name.emplace(name, _request(name, timeout)).first->second;
  }
  if(!has_req)
    pending.req->run();

  if(pending.datap->wait(err, schema) && schema) {
    auto ts = Time::now_ms();
    Core::MutexSptd::scope lock(m_mutex);
    m_track.emplace(schema->cid, ts);
    _replace(schema);
    m_pending_name.erase(name);
  } else if(!schema && !err) {
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
Schemas::get(int& err, const DB::Schemas::SelectorPatterns& patterns,
             std::vector<DB::Schema::Ptr>& schemas, uint32_t timeout) {
  _request(err, patterns, schemas, timeout);

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
Schemas::get(int& err, const DB::Schemas::SelectorPatterns& patterns,
             uint32_t timeout) {
  std::vector<DB::Schema::Ptr> schemas;
  get(err, patterns, schemas, timeout);
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


Schemas::Pending
Schemas::_request(cid_t cid, uint32_t timeout) {
  switch(_clients->flags) {
    case Clients::Flag::DEFAULT |Clients::Flag::BROKER |Clients::Flag::SCHEMA:
    case Clients::Flag::BROKER: {
      auto req = Comm::Protocol::Bkr::Req::ColumnGet<ColumnGetData>::make(
        Comm::Protocol::Mngr::Params::ColumnGetReq(
          Comm::Protocol::Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_ID,
          cid
        ),
        timeout,
        _clients->shared()
      );
      return Schemas::Pending(req, &req->data);
    }
    default: {
      auto req = Comm::Protocol::Mngr::Req::ColumnGet<ColumnGetData>::make(
        Comm::Protocol::Mngr::Params::ColumnGetReq(
          Comm::Protocol::Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_ID,
          cid
        ),
        timeout,
        _clients->shared()
      );
      return Schemas::Pending(req, &req->data);
    }
  }
}

Schemas::Pending
Schemas::_request(const std::string& name, uint32_t timeout) {
  switch(_clients->flags) {
    case Clients::Flag::DEFAULT |Clients::Flag::BROKER |Clients::Flag::SCHEMA:
    case Clients::Flag::BROKER: {
      auto req = Comm::Protocol::Bkr::Req::ColumnGet<ColumnGetData>::make(
        Comm::Protocol::Mngr::Params::ColumnGetReq(
          Comm::Protocol::Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME,
          name
        ),
        timeout,
        _clients->shared()
      );
      return Schemas::Pending(req, &req->data);
    }
    default: {
      auto req = Comm::Protocol::Mngr::Req::ColumnGet<ColumnGetData>::make(
        Comm::Protocol::Mngr::Params::ColumnGetReq(
          Comm::Protocol::Mngr::Params::ColumnGetReq::Flag::SCHEMA_BY_NAME,
          name
        ),
        timeout,
        _clients->shared()
      );
      return Schemas::Pending(req, &req->data);
    }
  }
}

void Schemas::_request(int& err,
                       const DB::Schemas::SelectorPatterns& patterns,
                       std::vector<DB::Schema::Ptr>& schemas,
                       uint32_t timeout) {
  Comm::Protocol::Mngr::Params::ColumnListReq params(patterns);
  switch(_clients->flags) {
    case Clients::Flag::DEFAULT |Clients::Flag::BROKER |Clients::Flag::SCHEMA:
    case Clients::Flag::BROKER:
      return Comm::Protocol::Bkr::Req::ColumnList_Sync::request(
        params, timeout, _clients->shared(), err, schemas);
    default:
      return Comm::Protocol::Mngr::Req::ColumnList_Sync::request(
        params, timeout, _clients->shared(), err, schemas);
  }
}


}}

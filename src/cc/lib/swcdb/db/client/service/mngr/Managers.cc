/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/service/mngr/Managers.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Cells/KeyComparator.h"

namespace SWC { namespace client {


Managers::Managers(const Config::Settings& settings,
                   Comm::IoContextPtr ioctx,
                   const ContextManager::Ptr& mngr_ctx)
    : queues(new Comm::client::ConnQueues(
        Comm::client::Serialized::Ptr(new Comm::client::Serialized(
          settings,
          "MANAGER",
          ioctx,
          mngr_ctx
            ? mngr_ctx
            : ContextManager::Ptr(new ContextManager(settings))
        )),
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.Mngr.connection.timeout"),
        settings.get<Config::Property::Value_uint16_g>(
          "swc.client.Mngr.connection.probes"),
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.Mngr.connection.keepalive"),
        settings.get<Config::Property::Value_int32_g>(
          "swc.client.request.again.delay")
      )),
      groups(Mngr::Groups::Ptr(new Mngr::Groups(settings))->init()),
      master_ranges_cache(settings) {
}

bool Managers::put(const ClientsPtr& clients,
                   const cid_t& cid, Comm::EndPoints& endpoints,
                   const Comm::client::ConnQueue::ReqBase::Ptr& req) {
  if(endpoints.empty()) {
    groups->select(cid, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping() || !req->valid()) {
        req->handle_no_conn();
      } else {
        Comm::Protocol::Mngr::Req::MngrActive::make(
          clients, cid, req)->run();
      }
      return false;
    }
  }
  queues->get(endpoints)->put(req);
  return true;
}

bool Managers::put_role_schemas(
                   const ClientsPtr& clients,
                   Comm::EndPoints& endpoints,
                   const Comm::client::ConnQueue::ReqBase::Ptr& req) {
  if(endpoints.empty()) {
    groups->select(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(clients->stopping() || !req->valid()) {
        req->handle_no_conn();
      } else {
        Comm::Protocol::Mngr::Req::MngrActive::make(
          clients, DB::Types::MngrRole::SCHEMAS, req)->run();
      }
      return false;
    }
  }
  queues->get(endpoints)->put(req);
  return true;
}


void Managers::MasterRangesCache::Column::clear_expired() noexcept {
  int64_t ms = Time::now_ms() - expiry_ms->get();
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ) {
    if(it->ts > ms) {
      ++it;
    } else {
      erase(it);
    }
  }
}

void Managers::MasterRangesCache::Column::remove(const rid_t rid) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it) {
    if(rid == it->rid) {
      erase(it);
      return;
    }
  }
}

void Managers::MasterRangesCache::Column::set(
        const rid_t rid,
        const DB::Cell::Key& range_begin,
        const DB::Cell::Key& range_end,
        const Comm::EndPoints& endpoints,
        const int64_t revision) {
  int64_t ts = Time::now_ms();
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it) {
    if(rid == it->rid) {
      erase(it);
      break;
    }
  }
  for(auto it = begin(); it != cend(); ++it) {
    switch(DB::KeySeq::compare(key_seq, range_end, it->key_end)) {
      case Condition::EQ:
        it->change(ts, rid, range_begin, range_end, endpoints, revision);
        return;
      case Condition::GT:
        insert(it, ts, rid, range_begin, range_end, endpoints, revision);
        return;
      default:
        break;
    }
  }
  push_back(ts, rid, range_begin, range_end, endpoints, revision);
}

bool Managers::MasterRangesCache::Column::get_read(
        const DB::Cell::Key& range_begin,
        const DB::Cell::Key& range_end,
        rid_t& rid,
        DB::Cell::Key& offset,
        bool& is_end,
        Comm::EndPoints& endpoints,
        int64_t& revision) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it) {
    if((it->key_begin.empty() || range_end.empty() ||
        DB::KeySeq::compare(key_seq, range_end, it->key_begin)
          != Condition::GT)
        &&
       (it->key_end.empty() || range_begin.empty() ||
        DB::KeySeq::compare(key_seq, range_begin, it->key_end)
          != Condition::LT) ) {
      if(Time::now_ms() - it->ts > expiry_ms->get())
        break;
      rid = it->rid;
      offset.copy(it->key_begin);
      endpoints = it->endpoints;
      revision = it->revision;
      is_end = it->key_end.empty();
      return true;
    }
  }
  return false;
}

bool Managers::MasterRangesCache::Column::get_write(
        const DB::Cell::Key& key,
        rid_t& rid,
        DB::Cell::Key& key_end,
        Comm::EndPoints& endpoints,
        int64_t& revision) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it) {
    // it->key_begin needs to be previous range_end (key Condition::GT)
    if(it->key_end.empty() || key.empty() ||
       DB::KeySeq::compare(key_seq, key, it->key_end)
        != Condition::LT ) {
      if(Time::now_ms() - it->ts > expiry_ms->get())
        break;
      rid = it->rid;
      key_end.copy(it->key_end);
      endpoints = it->endpoints;
      revision = it->revision;
      return true;
    }
  }
  return false;
}



}} //namespace SWC::client


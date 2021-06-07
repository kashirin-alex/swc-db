/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/rgr/Cache.h"
#include "swcdb/core/Time.h"

namespace SWC { namespace client {



CachedRangers::~CachedRangers() {
  for(auto c : *this) {
    for(auto r : c.second)
      delete r.second;
  }
}

void CachedRangers::clear() {
  Core::MutexSptd::scope lock(m_mutex);
  Map::clear();
}

void CachedRangers::clear_expired() {
  auto ms = Time::now_ms();

  Core::MutexSptd::scope lock(m_mutex);
  for(auto c = begin(); c != end(); ) {
    for(auto r = c->second.begin(); r != c->second.end(); ) {
      if(ms - r->second->ts > m_expiry_ms->get()) {
        delete r->second;
        r = c->second.erase(r);
      } else {
        ++r;
      }
    }
    if(c->second.empty())
      c = erase(c);
    else
      ++c;
  }
}

void CachedRangers::remove(const cid_t cid, const rid_t rid) {
  Core::MutexSptd::scope lock(m_mutex);

  auto c = find(cid);
  if(c == end())
    return;
  auto r = c->second.find(rid);
  if(r == c->second.end())
    return;
  delete r->second;
  c->second.erase(r);
  if(c->second.empty())
    erase(c);
}

bool CachedRangers::get(const cid_t cid, const rid_t rid,
                        Comm::EndPoints& endpoints) {
  bool found = false;

  Core::MutexSptd::scope lock(m_mutex);
  auto c = find(cid);
  if(c != end()) {
    auto r = c->second.find(rid);
    if(r != c->second.end() &&
       Time::now_ms() - r->second->ts < m_expiry_ms->get()) {
      found = true;
      endpoints = r->second->endpoints;
    }
  }
  return found;
}

void CachedRangers::set(const cid_t cid, const rid_t rid,
                        const Comm::EndPoints& endpoints) {
  auto r_new = new RangeEndPoints(Time::now_ms(), endpoints);

  Core::MutexSptd::scope lock(m_mutex);
  auto res = (*this)[cid].emplace(rid, r_new);
  if(!res.second) {
    delete res.first->second;
    res.first->second = r_new;
  }
}



}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/rgr/Cache.h"
#include "swcdb/core/Time.h"

namespace SWC { namespace client {



void CachedRangers::clear() {
  Core::MutexSptd::scope lock(m_mutex);
  Map::clear();
}

void CachedRangers::clear_expired() {
  auto ms = Time::now_ms();

  Core::MutexSptd::scope lock(m_mutex);
  for(auto c = begin(); c != cend(); ) {
    for(auto r = c->second.cbegin(); r != c->second.cend(); ) {
      if(ms - r->second.ts > m_expiry_ms->get()) {
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
  if(c == cend())
    return;
  auto r = c->second.find(rid);
  if(r == c->second.cend())
    return;
  c->second.erase(r);
  if(c->second.empty())
    erase(c);
}

bool CachedRangers::get(const cid_t cid, const rid_t rid,
                        Comm::EndPoints& endpoints) {
  Core::MutexSptd::scope lock(m_mutex);
  auto c = find(cid);
  if(c != cend()) {
    auto r = c->second.find(rid);
    if(r != c->second.cend() &&
       Time::now_ms() - r->second.ts < m_expiry_ms->get()) {
      endpoints = r->second.endpoints;
      return true;
    }
  }
  return false;
}

void CachedRangers::set(const cid_t cid, const rid_t rid,
                        const Comm::EndPoints& endpoints) {
  int64_t ts = Time::now_ms();
  Core::MutexSptd::scope lock(m_mutex);
  auto& r = (*this)[cid][rid];
  r.ts = ts;
  r.endpoints = endpoints;
}



}}

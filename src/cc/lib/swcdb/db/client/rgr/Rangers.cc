/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */
 

#include "swcdb/db/client/rgr/Rangers.h"
#include "swcdb/core/Time.h"

namespace SWC { namespace client {

Rangers::Rangers(const Property::V_GINT32::Ptr expiry_ms) 
                : m_expiry_ms(expiry_ms) { 
}

Rangers::~Rangers() { }
  
void Rangers::clear() {
  std::scoped_lock lock(m_mutex); 
  m_map.clear();
}
  
void Rangers::clear_expired() {
  auto ms = Time::now_ms();

  std::scoped_lock lock(m_mutex);     
  for(auto c = m_map.begin(); c != m_map.end(); ) {
    for(auto r = c->second.begin(); r != c->second.end(); ) {
      if(ms - r->second.ts > m_expiry_ms->get())
        r = c->second.erase(r);
      else
        ++r;
    }
    if(c->second.empty())
      c = m_map.erase(c);
    else
      ++c;
  }
}

void Rangers::remove(const int64_t cid, const int64_t rid) {
  std::scoped_lock lock(m_mutex);

  auto c = m_map.find(cid);
  if(c == m_map.end()) 
    return; 
  auto r = c->second.find(rid);
  if(r == c->second.end()) 
    return;
  c->second.erase(r);
  if(c->second.empty())
    m_map.erase(c);
}

const bool Rangers::get(const int64_t cid, const int64_t rid, 
                        EndPoints& endpoints) {
  bool found = false;

  std::scoped_lock lock(m_mutex);
  auto c = m_map.find(cid);
  if(c != m_map.end()) {
    auto r = c->second.find(rid);
    if(r != c->second.end() && 
       Time::now_ms() - r->second.ts < m_expiry_ms->get()) {
      found = true;
      endpoints.assign(
        r->second.endpoints.begin(), r->second.endpoints.end());
    }
  }
  return found;
}

void Rangers::set(const int64_t cid, const int64_t rid, 
                  const EndPoints& endpoints) {
  std::scoped_lock lock(m_mutex);
  auto c = m_map.find(cid);
  if(c == m_map.end()) {
    m_map[cid].emplace(rid, Rangers::Range(Time::now_ms(), endpoints));
  } else {
    auto r = c->second.find(rid);
    if(r == c->second.end()) {
      c->second.emplace(rid, Rangers::Range(Time::now_ms(), endpoints));
    } else {
      r->second = Rangers::Range(Time::now_ms(), endpoints);
    }
  }
}



}}
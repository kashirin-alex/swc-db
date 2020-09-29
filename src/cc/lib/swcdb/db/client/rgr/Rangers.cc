/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 

#include "swcdb/db/client/rgr/Rangers.h"
#include "swcdb/core/Time.h"

namespace SWC { namespace client {

Rangers::Rangers(const Config::Property::V_GINT32::Ptr expiry_ms) 
                : m_expiry_ms(expiry_ms) { 
}

Rangers::~Rangers() { 
  for(auto c : *this) {
    for(auto r : c.second)
      delete r.second;
  }
}
  
void Rangers::clear() {
  Mutex::scope lock(m_mutex);
  Map::clear();
}
  
void Rangers::clear_expired() {
  auto ms = Time::now_ms();

  Mutex::scope lock(m_mutex);
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

void Rangers::remove(const cid_t cid, const rid_t rid) {
  Mutex::scope lock(m_mutex);

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

bool Rangers::get(const cid_t cid, const rid_t rid, 
                  Comm::EndPoints& endpoints) {
  bool found = false;

  Mutex::scope lock(m_mutex);
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

void Rangers::set(const cid_t cid, const rid_t rid, 
                  const Comm::EndPoints& endpoints) {
  auto r_new = new RangeEndPoints(Time::now_ms(), endpoints);

  Mutex::scope lock(m_mutex);
  auto c = find(cid);
  if(c == end()) {
    (*this)[cid].emplace(rid, r_new);
  } else {
    auto r = c->second.find(rid);
    if(r == c->second.end()) {
      c->second.emplace(rid, r_new);
    } else {
      delete r->second;
      r->second = r_new;
    }
  }
}



}}
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_Rangers_h
#define swc_client_Rangers_h

#include "swcdb/core/Time.h"

namespace SWC { namespace client {

class Rangers  {
  public:

  Rangers(const gInt32tPtr expiry_ms) : m_expiry_ms(expiry_ms) { }

  virtual ~Rangers(){ }
  
  void clear() {
    std::scoped_lock lock(m_mutex); 
    m_map.clear();
  }
  
  void clear_expired() {
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

  void remove(const int64_t cid, const int64_t rid) {
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

  const bool get(const int64_t cid, const int64_t rid, EndPoints& endpoints) {
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

  void set(const int64_t cid, const int64_t rid, const EndPoints& endpoints) {
    std::scoped_lock lock(m_mutex);
    auto c = m_map.find(cid);
    if(c == m_map.end()) {
      m_map.insert(
        std::make_pair(
          cid, 
          (Ranges){{rid, Range(Time::now_ms(), endpoints) }} ));
    } else {
      auto r = c->second.find(rid);
      if(r == c->second.end()) {
        c->second.insert(
          std::make_pair(
            rid, 
            Range(Time::now_ms(), endpoints) ));
      } else {
        r->second = Range(Time::now_ms(), endpoints);
      }
    }
  }

  private:
  struct Range final {
    int64_t   ts;
    EndPoints endpoints;
    Range(const int64_t ts, const EndPoints endpoints)
          : ts(ts), endpoints(endpoints) {
    }
  };
  typedef std::unordered_map<int64_t, Range>  Ranges;
  std::mutex                                  m_mutex;
  std::unordered_map<int64_t, Ranges>         m_map;
  gInt32tPtr                                  m_expiry_ms;
  
};



}}

#endif // swc_client_Rangers_h
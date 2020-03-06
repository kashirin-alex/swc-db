/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_Rangers_h
#define swc_client_Rangers_h

#include <mutex>
#include <unordered_map>

#include "swcdb/client/Settings.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC { namespace client {

class Rangers  {
  public:

  Rangers(const Property::V_GINT32::Ptr expiry_ms);

  virtual ~Rangers();
  
  void clear();
  
  void clear_expired();

  void remove(const int64_t cid, const int64_t rid);

  const bool get(const int64_t cid, const int64_t rid, EndPoints& endpoints);

  void set(const int64_t cid, const int64_t rid, const EndPoints& endpoints);

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
  Property::V_GINT32::Ptr                     m_expiry_ms;
  
};



}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/client/Rangers.cc"
#endif 

#endif // swc_client_Rangers_h
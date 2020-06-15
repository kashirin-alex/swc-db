/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_client_rgr_Rangers_h
#define swc_db_client_rgr_Rangers_h

#include <mutex>
#include <unordered_map>

#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/client/Settings.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC { namespace client {

struct RangeEndPoints final {
  const int64_t     ts;
  const EndPoints   endpoints;
  RangeEndPoints(const int64_t ts, const EndPoints& endpoints)
                : ts(ts), endpoints(endpoints) {
  }
};


class Rangers final 
    : private std::unordered_map<
        cid_t, std::unordered_map<rid_t, RangeEndPoints*>> {
  public:

  Rangers(const Property::V_GINT32::Ptr expiry_ms);

  virtual ~Rangers();
  
  void clear();
  
  void clear_expired();

  void remove(const cid_t cid, const rid_t rid);

  bool get(const cid_t cid, const rid_t rid, EndPoints& endpoints);

  void set(const cid_t cid, const rid_t rid, const EndPoints& endpoints);

  private:

  Mutex                     m_mutex;
  Property::V_GINT32::Ptr   m_expiry_ms;
  
};



}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/rgr/Rangers.cc"
#endif 

#endif // swc_db_client_rgr_Rangers_h
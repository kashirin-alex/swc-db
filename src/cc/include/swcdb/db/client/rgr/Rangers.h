/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_rgr_Rangers_h
#define swcdb_db_client_rgr_Rangers_h

#include <mutex>
#include <unordered_map>

#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/client/Settings.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC { namespace client {

struct RangeEndPoints final {
  const int64_t           ts;
  const Comm::EndPoints   endpoints;
  RangeEndPoints(const int64_t ts, const Comm::EndPoints& endpoints)
                : ts(ts), endpoints(endpoints) {
  }
};


class Rangers final 
    : private std::unordered_map<
        cid_t, std::unordered_map<rid_t, RangeEndPoints*>> {
  
  typedef std::unordered_map<
    cid_t, std::unordered_map<rid_t, RangeEndPoints*>> Map;
    
  public:

  Rangers(const Config::Property::V_GINT32::Ptr expiry_ms);

  virtual ~Rangers();
  
  void clear();
  
  void clear_expired();

  void remove(const cid_t cid, const rid_t rid);

  bool get(const cid_t cid, const rid_t rid, Comm::EndPoints& endpoints);

  void set(const cid_t cid, const rid_t rid, 
           const Comm::EndPoints& endpoints);

  private:

  Mutex                             m_mutex;
  Config::Property::V_GINT32::Ptr   m_expiry_ms;
  
};



}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/rgr/Rangers.cc"
#endif 

#endif // swcdb_db_client_rgr_Rangers_h
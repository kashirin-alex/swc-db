/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_client_Schemas_h
#define swc_db_client_Schemas_h

#include "swcdb/db/client/Settings.h"
#include "swcdb/db/Columns/Schemas.h"

namespace SWC { namespace client {

class Schemas final : private DB::Schemas {
  public:
  
  typedef std::shared_ptr<Schemas> Ptr;

  Schemas(const Property::V_GINT32::Ptr expiry_ms);

  ~Schemas();
  
  void remove(cid_t cid);

  void remove(const std::string& name);

  DB::Schema::Ptr get(int& err, cid_t cid);
  
  DB::Schema::Ptr get(int& err, const std::string& name);

  private:

  void _request(int& err, cid_t cid, DB::Schema::Ptr& schema);

  void _request(int& err, const std::string& name, DB::Schema::Ptr& schema);

  std::unordered_map<cid_t, uint64_t>  m_track; // .second {time,queue(promises)}
  Property::V_GINT32::Ptr              m_expiry_ms;
  
};



}}

#endif // swc_db_client_Schemas_h
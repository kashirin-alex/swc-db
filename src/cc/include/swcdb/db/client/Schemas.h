/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_Schemas_h
#define swc_client_Schemas_h

#include "swcdb/db/client/Settings.h"
#include "swcdb/db/Columns/Schemas.h"

namespace SWC { namespace client {

class Schemas  {
  public:
  
  typedef std::shared_ptr<Schemas> Ptr;

  Schemas(const Property::V_GINT32::Ptr expiry_ms);

  virtual ~Schemas();
  
  void remove(int64_t cid);

  void remove(const std::string &name);

  DB::Schema::Ptr get(int& err, int64_t cid);
  
  DB::Schema::Ptr get(int& err, const std::string &name);

  void request(int& err, int64_t cid);

  void request(int& err, const std::string &name);

  private:
  std::mutex                            m_mutex;
  std::unordered_map<int64_t, uint64_t> m_track; // .second {time,queue(promises)}
  std::shared_ptr<DB::Schemas>          m_schemas = nullptr;
  Property::V_GINT32::Ptr               m_expiry_ms;
  
};



}}

#endif // swc_client_Schemas_h
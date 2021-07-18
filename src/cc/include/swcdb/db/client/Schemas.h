/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Schemas_h
#define swcdb_db_client_Schemas_h

#include "swcdb/db/client/Settings.h"
#include "swcdb/db/Columns/Schemas.h"

namespace SWC { namespace client {

class Schemas final : private DB::Schemas {
  public:

  SWC_CAN_INLINE
  Schemas(Clients* _clients,
          const Config::Property::V_GINT32::Ptr expiry_ms) noexcept
          : _clients(_clients), m_expiry_ms(expiry_ms) {
  }

  //~Schemas() { }

  void remove(cid_t cid);

  void remove(const std::string& name);

  DB::Schema::Ptr get(int& err, cid_t cid);

  DB::Schema::Ptr get(int& err, const std::string& name);

  DB::Schema::Ptr get(cid_t cid);

  DB::Schema::Ptr get(const std::string& name);

  void get(int& err, const DB::Schemas::NamePatterns& patterns,
           std::vector<DB::Schema::Ptr>& schemas);

  std::vector<DB::Schema::Ptr>
  get(int& err, const DB::Schemas::NamePatterns& patterns);

  void set(const DB::Schema::Ptr& schema);

  void set(const std::vector<DB::Schema::Ptr>& schemas);

  private:

  void _request(int& err, cid_t cid, DB::Schema::Ptr& schema);

  void _request(int& err, const std::string& name, DB::Schema::Ptr& schema);

  void _request(int& err,
                const DB::Schemas::NamePatterns& patterns,
                std::vector<DB::Schema::Ptr>& schemas);

  Clients*                            _clients;
  std::unordered_map<cid_t, int64_t>  m_track; // .second {time,queue(promises)}
  Config::Property::V_GINT32::Ptr     m_expiry_ms;

};



}}

#endif // swcdb_db_client_Schemas_h

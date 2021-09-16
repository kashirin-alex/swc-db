/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Schemas_h
#define swcdb_db_client_Schemas_h


#include "swcdb/db/client/Settings.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace client {


class Schemas final {
  public:

  SWC_CAN_INLINE
  Schemas(Clients* a_clients,
          const Config::Property::V_GINT32::Ptr expiry_ms) noexcept
          : _clients(a_clients), m_expiry_ms(expiry_ms) {
  }

  ~Schemas() { }

  void remove(cid_t cid);

  void remove(const std::string& name);

  void clear_expired();

  DB::Schema::Ptr get(int& err, cid_t cid,
                      uint32_t timeout=300000);

  DB::Schema::Ptr get(int& err, const std::string& name,
                      uint32_t timeout=300000);

  DB::Schema::Ptr get(cid_t cid);

  DB::Schema::Ptr get(const std::string& name);

  void get(int& err, const DB::Schemas::SelectorPatterns& patterns,
           DB::SchemasVec& schemas, uint32_t timeout=300000);

  DB::SchemasVec get(int& err, const DB::Schemas::SelectorPatterns& patterns,
                     uint32_t timeout=300000);

  void set(const DB::Schema::Ptr& schema);

  void set(const DB::SchemasVec& schemas);

  private:

  class ColumnGetData;
  struct Pending final {
    Comm::DispatchHandler::Ptr  req;
    ColumnGetData*              datap;
    SWC_CAN_INLINE
    Pending() noexcept : datap(nullptr) { }
    SWC_CAN_INLINE
    Pending(const Comm::DispatchHandler::Ptr& a_req, ColumnGetData* a_datap)
            noexcept : req(a_req), datap(a_datap) { }
    SWC_CAN_INLINE
    ~Pending() { }
  };

  Pending _request(cid_t cid, uint32_t timeout);

  Pending _request(const std::string& name, uint32_t timeout);

  void _request(int& err, const DB::Schemas::SelectorPatterns& patterns,
                DB::SchemasVec& schemas, uint32_t timeout);

  struct SchemaData {
    int64_t         ts;
    DB::Schema::Ptr schema;
    SWC_CAN_INLINE
    ~SchemaData() { }
    SWC_CAN_INLINE
    void assign(int64_t _ts, const DB::Schema::Ptr& _schema) {
      ts = _ts;
      schema = _schema;
    }
  };
  Core::MutexSptd                           m_mutex;
  std::unordered_map<cid_t, SchemaData>     m_schemas;
  Clients*                                  _clients;
  Config::Property::V_GINT32::Ptr           m_expiry_ms;
  std::unordered_map<cid_t,       Pending>  m_pending_cid;
  std::unordered_map<std::string, Pending>  m_pending_name;

};



}}

#endif // swcdb_db_client_Schemas_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnGet_Sync_h
#define swcdb_db_protocol_mngr_req_ColumnGet_Sync_h


#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnGet_Sync: public ColumnGet_Base {
  public:

  static void schema(const SWC::client::Clients::Ptr& clients,
                     int& err, const std::string& name,
                     DB::Schema::Ptr& _schema,
                     const uint32_t timeout = 10000);

  static void schema(const SWC::client::Clients::Ptr& clients,
                     int& err, cid_t cid,
                     DB::Schema::Ptr& _schema,
                     const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnGetReq& params,
                      int& err, DB::Schema::Ptr& _schema,
                      const uint32_t timeout = 10000);

  std::promise<void> await;

  ColumnGet_Sync(const SWC::client::Clients::Ptr& clients,
                 const Params::ColumnGetReq& params,
                 int& err, DB::Schema::Ptr& _schema,
                 const uint32_t timeout);

  virtual ~ColumnGet_Sync() { }

  protected:
  virtual void callback(int err, const Params::ColumnGetRsp& rsp) override;

  private:
  int&                      err;
  DB::Schema::Ptr&          _schema;
};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Sync.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnGet_Sync_h

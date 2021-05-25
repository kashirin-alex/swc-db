/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnGet_Sync_h
#define swcdb_db_protocol_bkr_req_ColumnGet_Sync_h


#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnGet_Sync: public client::ConnQueue::ReqBase {
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
                      const Mngr::Params::ColumnGetReq& params,
                      int& err, DB::Schema::Ptr& _schema,
                      const uint32_t timeout = 10000);

  std::promise<void> await;

  ColumnGet_Sync(const SWC::client::Clients::Ptr& clients,
                 const Mngr::Params::ColumnGetReq& params,
                 int& err, DB::Schema::Ptr& _schema,
                 const uint32_t timeout);

  virtual ~ColumnGet_Sync() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  SWC::client::Clients::Ptr clients;
  int&                      err;
  DB::Schema::Ptr&          _schema;
  size_t                    bkr_idx;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnGet_Sync.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnGet_Sync_h

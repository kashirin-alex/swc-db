/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnMng_Sync_h
#define swcdb_db_protocol_mngr_req_ColumnMng_Sync_h


#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnMng_Sync: public ColumnMng_Base {
  public:

  static void create(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, int& err,
                     const uint32_t timeout = 10000);

  static void modify(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, int& err,
                     const uint32_t timeout = 10000);

  static void remove(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, int& err,
                     const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      Params::ColumnMng::Function func,
                      const DB::Schema::Ptr& schema, int& err,
                      const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnMng& params, int& err,
                      const uint32_t timeout = 10000);

  std::promise<void> await;

  ColumnMng_Sync(const SWC::client::Clients::Ptr& clients,
                 const Params::ColumnMng& params, int& err,
                 const uint32_t timeout);

  virtual ~ColumnMng_Sync() { }

  protected:
  virtual void callback(int err) override;

  private:
  int& err;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Sync.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnMng_Sync_h

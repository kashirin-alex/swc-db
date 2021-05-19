/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnMng_h
#define swcdb_db_protocol_mngr_req_ColumnMng_h


#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnMng: public client::ConnQueue::ReqBase {
  public:

  using Func = Params::ColumnMng::Function;
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             int)> Cb_t;

  static void create(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void modify(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void remove(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      Func func,
                      const DB::Schema::Ptr& schema, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::ColumnMng& params, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  ColumnMng(const SWC::client::Clients::Ptr& clients,
            const Params::ColumnMng& params, Cb_t&& cb,
            const uint32_t timeout);

  virtual ~ColumnMng() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  void clear_endpoints();

  SWC::client::Clients::Ptr clients;
  const Cb_t                cb;
  EndPoints                 endpoints;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnMng_h

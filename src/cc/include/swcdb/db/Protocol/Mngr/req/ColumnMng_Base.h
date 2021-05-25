/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnMng_Base_h
#define swcdb_db_protocol_mngr_req_ColumnMng_Base_h


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnMng_Base: public client::ConnQueue::ReqBase {
  public:

  using Func = Params::ColumnMng::Function;

  ColumnMng_Base(const SWC::client::Clients::Ptr& clients,
                 const Params::ColumnMng& params,
                 const uint32_t timeout);

  virtual ~ColumnMng_Base() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  protected:

  virtual void callback(int error) = 0;

  private:

  void clear_endpoints();

  SWC::client::Clients::Ptr clients;
  EndPoints                 endpoints;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Base.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnMng_Base_h

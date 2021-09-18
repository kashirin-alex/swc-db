/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RgrGet_Base_h
#define swcdb_db_protocol_req_RgrGet_Base_h


#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class RgrGet_Base : public client::ConnQueue::ReqBase {
  public:

  SWC_CAN_INLINE
  RgrGet_Base(const Params::RgrGetReq& params, const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  Buffers::make(params, 0 ,RGR_GET, timeout)),
                cid(params.cid) {
  }

  virtual ~RgrGet_Base() noexcept { }

  void handle_no_conn() override;

  bool run() override {
    return get_clients()->managers.put(get_clients(), cid, endpoints, req());
  }

  protected:

  virtual SWC::client::Clients::Ptr& get_clients() noexcept = 0;

  virtual void callback(Params::RgrGetRsp&) = 0;

  private:

  EndPoints                 endpoints;
  cid_t                     cid;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/RgrGet_Base.cc"
#endif

#endif // swcdb_db_protocol_req_RgrGet_Base_h

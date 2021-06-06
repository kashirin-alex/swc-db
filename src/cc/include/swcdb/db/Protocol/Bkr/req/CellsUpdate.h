/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_CellsUpdate_h
#define swcdb_db_protocol_bkr_req_CellsUpdate_h



#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class CellsUpdate: public client::ConnQueue::ReqBase {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const Params::CellsUpdateRsp&)> Cb_t;

  static void
  request(const SWC::client::Clients::Ptr& clients,
          const EndPoints& endpoints,
          cid_t cid,
          const DynamicBuffer::Ptr& buffer,
          Cb_t&& cb,
          const uint32_t timeout = 10000);


  CellsUpdate(const SWC::client::Clients::Ptr& clients,
              const EndPoints& endpoints,
              Buffers::Ptr&& cbp,
              Cb_t&& cb);

  virtual ~CellsUpdate() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:
  SWC::client::Clients::Ptr clients;
  EndPoints                 endpoints;
  const Cb_t                cb;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/CellsUpdate.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_CellsUpdate_h

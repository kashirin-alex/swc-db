
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_req_ColumnCompact_h
#define swc_db_protocol_req_ColumnCompact_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnCompact: public Comm::client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const Params::ColumnCompactRsp&)> Cb_t;
 
  static void request(cid_t cid, const Cb_t& cb, 
                      const uint32_t timeout = 10000);

  static void request(const Params::ColumnCompactReq& params,
                             const Cb_t& cb, const uint32_t timeout = 10000);

  ColumnCompact(const Params::ColumnCompactReq& params, const Cb_t& cb, 
                const uint32_t timeout);

  virtual ~ColumnCompact();

  void handle_no_conn() override;

  bool run() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:
  
  void clear_endpoints();

  const Cb_t   cb;
  const cid_t  cid;
  Comm::EndPoints    endpoints;
};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.cc"
#endif 

#endif // swc_db_protocol_req_ColumnCompact_h

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RgrGet_h
#define swcdb_db_protocol_req_RgrGet_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {

  
class RgrGet: public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&, 
                             const Params::RgrGetRsp&)> Cb_t;
 
  static void request(cid_t cid, rid_t rid, bool next_range,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  static void request(const Params::RgrGetReq& params,
                      Cb_t&& cb, const uint32_t timeout = 10000);

  static Ptr make(const Params::RgrGetReq& params,
                  Cb_t&& cb, const uint32_t timeout = 10000);

  RgrGet(const Params::RgrGetReq& params, Cb_t&& cb, 
         const uint32_t timeout);

  virtual ~RgrGet();

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:
  
  void clear_endpoints();

  const Cb_t        cb;
  EndPoints         endpoints;
  cid_t             cid;
};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/RgrGet.cc"
#endif 

#endif // swcdb_db_protocol_req_RgrGet_h

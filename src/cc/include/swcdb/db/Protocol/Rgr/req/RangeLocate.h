
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_rgr_req_RangeLocate_h
#define swc_db_protocol_rgr_req_RangeLocate_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

  
class RangeLocate: public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&, 
                             const Params::RangeLocateRsp&)> Cb_t;

  static void request(const Params::RangeLocateReq& params,
                             const EndPoints& endpoints, 
                             const Cb_t& cb, const uint32_t timeout = 10000);

  RangeLocate(const Params::RangeLocateReq& params, 
              const EndPoints& endpoints,
              const Cb_t& cb, const uint32_t timeout);

  virtual ~RangeLocate();

  void handle_no_conn() override;

  bool run(uint32_t timeout=0) override;

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  const EndPoints  endpoints;
  const Cb_t       cb;

};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.cc"
#endif 

#endif // swc_db_protocol_rgr_req_RangeLocate_h

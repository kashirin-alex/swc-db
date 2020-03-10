
/*
 * Copyright (C) 2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_req_ColumnCompact_h
#define swc_db_protocol_req_ColumnCompact_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnCompact.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnCompact: public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(client::ConnQueue::ReqBase::Ptr, 
                             const Params::ColumnCompactRsp&)> Cb_t;
 
  static void request(int64_t cid, const Cb_t cb, 
                      const uint32_t timeout = 10000);

  static void request(const Params::ColumnCompactReq params,
                             const Cb_t cb, const uint32_t timeout = 10000);

  ColumnCompact(const Params::ColumnCompactReq& params, const Cb_t cb, 
                const uint32_t timeout);

  virtual ~ColumnCompact();

  void handle_no_conn() override;

  bool run(uint32_t timeout=0) override;

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  private:
  
  void clear_endpoints();

  const Cb_t      cb;
  const int64_t   cid;
  EndPoints       endpoints;
};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.cc"
#endif 

#endif // swc_db_protocol_req_ColumnCompact_h

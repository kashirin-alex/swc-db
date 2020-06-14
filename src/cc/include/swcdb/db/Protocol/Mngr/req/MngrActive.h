
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_mngr_req_MngrActive_h
#define swc_db_protocol_mngr_req_MngrActive_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"
#include "swcdb/db/client/mngr/Groups.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {


class MngrActive : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<MngrActive> Ptr;

  const uint8_t role;
  const cid_t   cid;

  static Ptr make(const cid_t& cid, const DispatchHandler::Ptr& hdlr,
                  uint32_t timeout_ms=60000);

  static Ptr make(const uint8_t& role, const DispatchHandler::Ptr& hdlr,
                  uint32_t timeout_ms=60000);

  MngrActive(const uint8_t& role, const cid_t& cid, 
             const DispatchHandler::Ptr& hdlr, uint32_t timeout_ms);

  virtual ~MngrActive();

  void run_within(uint32_t t_ms = 1000);

  void handle_no_conn() override;

  bool run(uint32_t timeout=0) override;

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  private:
  DispatchHandler::Ptr            hdlr;
  int                             nxt;
  client::Mngr::Hosts             hosts;
  client::Mngr::Groups::GroupHost group_host;
  asio::high_resolution_timer     timer;

  protected:
  const uint32_t  timeout_ms;
};

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/MngrActive.cc"
#endif 

#endif // swc_db_protocol_mngr_req_MngrActive_h

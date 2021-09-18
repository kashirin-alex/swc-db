/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_MngrActive_h
#define swcdb_db_protocol_mngr_req_MngrActive_h


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"
#include "swcdb/db/client/service/mngr/Groups.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class MngrActive : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<MngrActive> Ptr;

  SWC_CAN_INLINE
  static Ptr make(const SWC::client::Clients::Ptr& clients,
                  const cid_t& cid, const DispatchHandler::Ptr& hdlr,
                  uint32_t timeout_ms=60000) {
    return Ptr(new MngrActive(
      clients, DB::Types::MngrRole::COLUMNS, cid, hdlr, timeout_ms));
  }

  SWC_CAN_INLINE
  static Ptr make(const SWC::client::Clients::Ptr& clients,
                  const uint8_t& role, const DispatchHandler::Ptr& hdlr,
                  uint32_t timeout_ms=60000) {
    return Ptr(new MngrActive(
      clients, role, DB::Schema::NO_CID, hdlr, timeout_ms));
  }

  SWC_CAN_INLINE
  MngrActive(const SWC::client::Clients::Ptr& a_clients,
             const uint8_t& a_role, const cid_t& a_cid,
             const DispatchHandler::Ptr& a_hdlr, uint32_t a_timeout_ms)
            : client::ConnQueue::ReqBase(
                Buffers::make(
                  Params::MngrActiveReq(a_role, a_cid),
                  0, MNGR_ACTIVE, a_timeout_ms
                )
              ),
              clients(a_clients),
              role(a_role), cid(a_cid), hdlr(a_hdlr), nxt(0),
              timer(asio::high_resolution_timer(
                      clients->get_mngr_io()->executor())),
              timeout_ms(a_timeout_ms) {
  }

  virtual ~MngrActive() noexcept { }

  void run_within(uint32_t t_ms = 1000);

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:
  SWC::client::Clients::Ptr             clients;
  const uint8_t                         role;
  const cid_t                           cid;
  DispatchHandler::Ptr                  hdlr;
  size_t                                nxt;
  SWC::client::Mngr::Hosts              hosts;
  SWC::client::Mngr::Groups::GroupHost  group_host;
  asio::high_resolution_timer           timer;

  protected:
  const uint32_t                        timeout_ms;
};

}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/MngrActive.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_MngrActive_h

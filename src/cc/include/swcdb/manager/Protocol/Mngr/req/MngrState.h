/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_req_MngrState_h
#define swcdb_manager_Protocol_mngr_req_MngrState_h

#include "swcdb/manager/Protocol/Mngr/params/MngrState.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr {namespace Req {


class MngrState : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<MngrState> Ptr;

  MngrState(const ResponseCallback::Ptr& cb,
            const Manager::MngrsStatus& states,
            uint64_t token,
            const EndPoint& mngr_host,
            uint32_t timeout);

  MngrState(MngrState&&) = delete;
  MngrState(const MngrState&) = delete;
  MngrState& operator=(MngrState&&) = delete;
  MngrState& operator=(const MngrState&) = delete;

  virtual ~MngrState() noexcept { }

  bool insistent() noexcept override { return true; }

  void handle_no_conn() override { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:
  ResponseCallback::Ptr   cb;

};

}}}}}

#endif // swcdb_manager_Protocol_mngr_req_MngrState_h

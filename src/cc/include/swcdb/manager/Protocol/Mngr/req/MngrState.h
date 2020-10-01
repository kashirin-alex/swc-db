
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_manager_Protocol_mngr_req_MngrState_h
#define swcdb_manager_Protocol_mngr_req_MngrState_h

#include "swcdb/manager/Protocol/Mngr/params/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


class MngrState : public Comm::client::ConnQueue::ReqBase {
  public:

  MngrState(const Comm::ResponseCallback::Ptr& cb, 
            const Manager::MngrsStatus& states, 
            uint64_t token, const Comm::EndPoint& mngr_host, 
            uint32_t timeout);
  
  virtual ~MngrState();

  void disconnected(const Comm::ConnHandlerPtr& conn);

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:
  Comm::ResponseCallback::Ptr   cb;
};

}}}}

#endif // swcdb_manager_Protocol_mngr_req_MngrState_h

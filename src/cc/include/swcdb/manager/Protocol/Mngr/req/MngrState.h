
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_manager_Protocol_mngr_req_MngrState_h
#define swc_manager_Protocol_mngr_req_MngrState_h

#include "swcdb/manager/Protocol/Mngr/params/MngrState.h"
 
namespace SWC { namespace Protocol { namespace Mngr {namespace Req {


class MngrState : public client::ConnQueue::ReqBase {
  public:

  MngrState(ResponseCallback::Ptr cb, Manager::MngrsStatus &states, 
            uint64_t token, const EndPoint& mngr_host, uint32_t timeout);
  
  virtual ~MngrState();

  void disconnected(ConnHandlerPtr conn);

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  private:
  ResponseCallback::Ptr   cb;
};

}}}}

#endif // swc_manager_Protocol_mngr_req_MngrState_h

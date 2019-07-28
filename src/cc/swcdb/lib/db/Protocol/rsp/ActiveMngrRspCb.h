
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rsp_ActiveMngrRspCb_h
#define swc_lib_db_protocol_rsp_ActiveMngrRspCb_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"

namespace SWC {
namespace Protocol {
namespace Rsp {

class ActiveMngrRspCb : public DispatchHandler {
  public:

  ActiveMngrRspCb(client::ClientsPtr clients, 
                  DispatchHandlerPtr mngr_active)
                  : clients(clients), mngr_active(mngr_active) {}

  virtual ~ActiveMngrRspCb(){}

  virtual void run(EndPoints endpoints){
    std::cout << "ActiveMngrRspCb-RUN (virtual)\n";
  }

  
  DispatchHandlerPtr mngr_active;
  client::ClientsPtr clients;

};
typedef std::shared_ptr<ActiveMngrRspCb> ActiveMngrRspCbPtr;

}}}

#endif // swc_lib_db_protocol_rsp_ActiveMngrRspCb_h

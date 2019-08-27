
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_MngrUpdateRangeServers_h
#define swc_lib_db_protocol_req_MngrUpdateRangeServers_h

#include "swcdb/lib/db/Protocol/params/MngrUpdateRangeServers.h"

namespace SWC {
namespace Protocol {
namespace Req {

class MngrUpdateRangeServers : public DispatchHandler {
  public:

  static CommBufPtr get_buf(server::Mngr::RsStatusList &hosts, 
                            bool sync_all) {
    Params::MngrUpdateRangeServers params(hosts, sync_all);
    CommHeader header(Command::MNGR_UPDATE_RANGESERVERS, 60000);
    CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
    return cbp;
  }
  static void put(server::Mngr::RsStatusList &hosts, bool sync_all){
    put(get_buf(hosts, sync_all));
  }
  static void put(CommBufPtr cbp){
    Env::MngrRoleState::get()->req_mngr_inchain(
      [cbp] (client::ClientConPtr mngr) {        
        // std::cout << "[RS-status] update req_mngr_inchain \n";
        (std::make_shared<MngrUpdateRangeServers>(mngr, cbp))->run();
      }
    );
  }

  MngrUpdateRangeServers(client::ClientConPtr conn, CommBufPtr cbp)
                        : conn(conn), cbp(cbp), was_called(false) {
  }
  
  virtual ~MngrUpdateRangeServers() { }
  
  bool run(uint32_t timeout=60000) override {
    return conn->send_request(cbp, shared_from_this()) == Error::OK;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) {
    
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());
    
    if(ev->type == Event::Type::DISCONNECT){
      if(!was_called)
        put(cbp);
      return;
    }

    if(ev->header.command == Protocol::Command::MNGR_UPDATE_RANGESERVERS 
       && Protocol::response_code(ev) == Error::OK){
      was_called = true;
      return;
    }

    conn->do_close();
  }

  private:
  client::ClientConPtr  conn;
  CommBufPtr            cbp;
  std::atomic<bool>     was_called;
};

typedef std::shared_ptr<MngrUpdateRangeServers> MngrUpdateRangeServersPtr;

}}}

#endif // swc_lib_db_protocol_req_MngrUpdateRangeServers_h

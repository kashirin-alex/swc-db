/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Rmdir_h
#define swc_fs_Broker_Protocol_req_Rmdir_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rmdir.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Rmdir : public Base {

  public:

  Rmdir(uint32_t timeout, const std::string& name, 
        const Callback::RmdirCb_t& cb=0) 
        : name(name), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());

    cbp = CommBuf::make(Params::RmdirReq(name));
    cbp->header.set(Cmd::FUNCTION_RMDIR, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_RMDIR, &ptr, &remain))
      return;

    SWC_LOGF(LOG_DEBUG, "rmdir path='%s' error='%d'", name.c_str(), error);
    
    cb(error);
  }

  private:
  const std::string    name;
  Callback::RmdirCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Rmdir_h
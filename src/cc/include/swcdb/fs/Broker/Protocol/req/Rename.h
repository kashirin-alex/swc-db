/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Rename_h
#define swc_fs_Broker_Protocol_req_Rename_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Rename.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Rename : public Base {

  public:

  Rename(uint32_t timeout, const std::string& from, const std::string& to,
        const Callback::RenameCb_t& cb=0) 
        : from(from), to(to), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s'", from.c_str(), to.c_str());

    cbp = CommBuf::make(Params::RenameReq(from, to));
    cbp->header.set(Cmd::FUNCTION_RENAME, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_RENAME, &ptr, &remain))
      return;

    SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s' error='%d'", 
              from.c_str(), to.c_str(), error);
    cb(error);
  }

  private:
  const std::string     from;
  const std::string     to;
  Callback::RenameCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Rename_h
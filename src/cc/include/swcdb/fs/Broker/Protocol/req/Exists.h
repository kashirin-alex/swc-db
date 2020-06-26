/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Exists_h
#define swc_fs_Broker_Protocol_req_Exists_h

#include "swcdb/fs/Broker/Protocol/params/Exists.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Exists : public Base {

  public:

  bool  state;

  Exists(uint32_t timeout, const std::string& name, 
         const Callback::ExistsCb_t& cb=0) 
        : name(name), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "exists path='%s'", name.c_str());

    cbp = CommBuf::make(Params::ExistsReq(name));
    cbp->header.set(Cmd::FUNCTION_EXISTS, timeout);
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, bool state){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_EXISTS, &ptr, &remain))
      return;

    if(!error) {
      Params::ExistsRsp params;
      params.decode(&ptr, &remain);
      state = params.exists;
    } else {
      state = false;
    }

    SWC_LOGF(LOG_DEBUG, "exists path='%s' error='%d' state='%d'",
               name.c_str(), error, (int)state);
    
    cb(error, state);
  }

  private:
  const std::string     name;
  Callback::ExistsCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Exists_h
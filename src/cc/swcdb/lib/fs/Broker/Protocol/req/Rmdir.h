/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Rmdir_h
#define swc_lib_fs_Broker_Protocol_req_Rmdir_h

#include "Base.h"
#include "../params/Rmdir.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Rmdir : public Base {

  public:

  Rmdir(uint32_t timeout, const String &name, Callback::RmdirCb_t cb=0) 
        : name(name), cb(cb) {
    HT_DEBUGF("rmdir path='%s'", name.c_str());

    CommHeader header(Cmd::FUNCTION_RMDIR, timeout);
    Params::RmdirReq params(name);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length()));
    params.encode(cbp->get_data_ptr_address());
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_RMDIR, &ptr, &remain))
      return;

    HT_DEBUGF("rmdir path='%s' error='%d'", name.c_str(), error);
    
    cb(error);
  }

  private:
  const String          name;
  Callback::RmdirCb_t  cb;
};
typedef std::shared_ptr<Rmdir> RmdirPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Rmdir_h
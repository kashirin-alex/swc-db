/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Rename_h
#define swc_lib_fs_Broker_Protocol_req_Rename_h

#include "Base.h"
#include "../params/Rename.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Rename : public Base {

  public:

  Rename(uint32_t timeout, const std::string &from, const std::string &to,
        Callback::RenameCb_t cb=0) 
        : from(from), to(to), cb(cb) {
    HT_DEBUGF("rename '%s' to '%s'", from.c_str(), to.c_str());

    CommHeader header(Cmd::FUNCTION_RENAME, timeout);
    Params::RenameReq params(from, to);
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

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_RENAME, &ptr, &remain))
      return;

    HT_DEBUGF("rename '%s' to '%s' error='%d'", 
              from.c_str(), to.c_str(), error);
    cb(error);
  }

  private:
  const std::string     from;
  const std::string     to;
  Callback::RenameCb_t  cb;
};
typedef std::shared_ptr<Rename> RenamePtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Rename_h
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Rmdir.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {


Rmdir::Rmdir(uint32_t timeout, const std::string& name, 
             const FS::Callback::RmdirCb_t& cb) 
            : name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());

  cbp = Comm::Buffers::make(Params::RmdirReq(name));
  cbp->header.set(Cmd::FUNCTION_RMDIR, timeout);
}

std::promise<void> Rmdir::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int){ await->set_value(); };
  return r_promise;
}

void Rmdir::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_RMDIR, &ptr, &remain))
    return;

  SWC_LOGF(LOG_DEBUG, "rmdir path='%s' error='%d'", name.c_str(), error);
  
  cb(error);
}



}}}}

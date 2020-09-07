/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Rename.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Rename::Rename(uint32_t timeout, const std::string& from, const std::string& to,
               const Callback::RenameCb_t& cb) 
              : from(from), to(to), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s'", from.c_str(), to.c_str());

  cbp = CommBuf::make(Params::RenameReq(from, to));
  cbp->header.set(Cmd::FUNCTION_RENAME, timeout);
}

std::promise<void> Rename::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int){ await->set_value(); };
  return r_promise;
}

void Rename::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_RENAME, &ptr, &remain))
    return;

  SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s' error='%d'", 
            from.c_str(), to.c_str(), error);
  cb(error);
}



}}}}
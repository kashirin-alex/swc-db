/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Exists.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Exists::Exists(uint32_t timeout, const std::string& name, 
               const Callback::ExistsCb_t& cb) 
              : name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "exists path='%s'", name.c_str());

  cbp = Comm::Buffers::make(Params::ExistsReq(name));
  cbp->header.set(Cmd::FUNCTION_EXISTS, timeout);
}

std::promise<void> Exists::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, bool){ await->set_value(); };
  return r_promise;
}

void Exists::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_EXISTS, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::ExistsRsp params;
      params.decode(&ptr, &remain);
      state = params.exists;

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  } else {
    state = false;
  }

  SWC_LOGF(LOG_DEBUG, "exists path='%s' error='%d' state='%d'",
             name.c_str(), error, (int)state);
  
  cb(error, state);
}



}}}}

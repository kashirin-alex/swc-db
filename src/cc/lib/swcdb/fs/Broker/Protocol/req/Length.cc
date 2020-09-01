/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Length.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Length::Length(uint32_t timeout, const std::string& name, 
               const Callback::LengthCb_t& cb) 
              : length(0), name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "length path='%s'", name.c_str());

  cbp = CommBuf::make(Params::LengthReq(name));
  cbp->header.set(Cmd::FUNCTION_LENGTH, timeout);
}

std::promise<void> Length::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, size_t){ await->set_value(); };
  return r_promise;
}

void Length::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_LENGTH, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::LengthRsp params;
      params.decode(&ptr, &remain);
      length = params.length;

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOGF(LOG_DEBUG, "length path='%s' error='%d' length='%lu'",
           name.c_str(), error, length);
  
  cb(error, length);
}



}}}}

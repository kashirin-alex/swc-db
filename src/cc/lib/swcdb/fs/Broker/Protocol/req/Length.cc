/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Length.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Length::Length(uint32_t timeout, const std::string& name, 
               const FS::Callback::LengthCb_t& cb) 
              : length(0), name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "length path='%s'", name.c_str());

  cbp = Buffers::make(Params::LengthReq(name));
  cbp->header.set(FUNCTION_LENGTH, timeout);
}

std::promise<void> Length::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, size_t){ await->set_value(); };
  return r_promise;
}

void Length::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_LENGTH, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::LengthRsp params;
      params.decode(&ptr, &remain);
      length = params.length;

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOGF(LOG_DEBUG, "length path='%s' error='%d' length='%lu'",
           name.c_str(), error, length);
  
  cb(error, length);
}



}}}}}

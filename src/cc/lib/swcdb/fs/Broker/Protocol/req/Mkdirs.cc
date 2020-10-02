/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Mkdirs.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Mkdirs::Mkdirs(uint32_t timeout, const std::string& name, 
               const FS::Callback::MkdirsCb_t& cb) 
              : name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", name.c_str());

  cbp = Buffers::make(Params::MkdirsReq(name));
  cbp->header.set(FUNCTION_MKDIRS, timeout);
}

std::promise<void> Mkdirs::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int){ await->set_value(); };
  return r_promise;
}

void Mkdirs::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_MKDIRS, &ptr, &remain))
    return;

  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s' error='%d'", name.c_str(), error);
  
  cb(error);
}



}}}}}

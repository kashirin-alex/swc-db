/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Remove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Remove::Remove(uint32_t timeout, const std::string& name, 
               const FS::Callback::RemoveCb_t& cb) 
              : Base(Buffers::make(Params::RemoveReq(name))),
                name(name), cb(cb) {
  cbp->header.set(FUNCTION_REMOVE, timeout);
  SWC_LOGF(LOG_DEBUG, "remove path='%s'", name.c_str());
}

void Remove::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_REMOVE, &ptr, &remain))
    return;

  SWC_LOGF(LOG_DEBUG, "remove path='%s' error='%d'", name.c_str(), error);
  
  cb(error);
}



}}}}}

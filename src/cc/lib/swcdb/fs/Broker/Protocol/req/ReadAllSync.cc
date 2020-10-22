/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/ReadAllSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


ReadAllSync::ReadAllSync(uint32_t timeout, const std::string& name,
                         StaticBuffer* dst)
                        : Base(Buffers::make(Params::ReadAllReq(name))),
                          buffer(dst), name(name) {
  cbp->header.set(FUNCTION_READ_ALL, timeout);
  SWC_LOGF(LOG_DEBUG, "read-all timeout=%d %s", timeout, name.c_str());
}

void ReadAllSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_READ_ALL, &ptr, &remain))
    return;
    
  if(!error)
    buffer->set(ev->data_ext);

  SWC_LOGF(LOG_DEBUG, "read-all %s amount='%lu' error='%d'", 
                        name.c_str(), 
                        error ? 0 : buffer->size,
                        error);
  BaseSync::acknowledge();
}



}}}}}

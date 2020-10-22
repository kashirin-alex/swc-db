/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/RemoveSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


RemoveSync::RemoveSync(uint32_t timeout, const std::string& name) 
                      : Base(Buffers::make(Params::RemoveReq(name))), 
                        name(name) {
  cbp->header.set(FUNCTION_REMOVE, timeout);
  SWC_LOGF(LOG_DEBUG, "remove path='%s'", name.c_str());
}

void RemoveSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_remove(ev, name);
  BaseSync::acknowledge();
}



}}}}}

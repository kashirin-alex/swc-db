/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/RmdirSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


RmdirSync::RmdirSync(uint32_t timeout, const std::string& name)
                    : Base(Buffers::make(Params::RmdirReq(name))),
                      name(name) {
  cbp->header.set(FUNCTION_RMDIR, timeout);
  SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());
}


void RmdirSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_rmdir(ev, name);
  BaseSync::acknowledge();
}



}}}}}

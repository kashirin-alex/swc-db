/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/RenameSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


RenameSync::RenameSync(uint32_t timeout, 
                       const std::string& from, const std::string& to)
                      : Base(Buffers::make(Params::RenameReq(from, to))),
                        from(from), to(to) {
  cbp->header.set(FUNCTION_RENAME, timeout);
  SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s'", from.c_str(), to.c_str());
}

void RenameSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_rename(ev, from, to);
  BaseSync::acknowledge();
}



}}}}}
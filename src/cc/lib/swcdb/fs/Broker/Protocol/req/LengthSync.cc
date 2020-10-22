/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/LengthSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


LengthSync::LengthSync(uint32_t timeout, const std::string& name) 
                      : Base(Buffers::make(Params::LengthReq(name))),
                        length(0), name(name) {
  cbp->header.set(FUNCTION_LENGTH, timeout);
  SWC_LOGF(LOG_DEBUG, "length path='%s'", name.c_str());
}

void LengthSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_length(ev, name, length);
  BaseSync::acknowledge();
}



}}}}}

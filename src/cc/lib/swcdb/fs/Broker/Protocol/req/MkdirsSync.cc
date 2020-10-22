/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/MkdirsSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


MkdirsSync::MkdirsSync(uint32_t timeout, const std::string& name) 
                      : Base(Buffers::make(Params::MkdirsReq(name))),
                        name(name) {
  cbp->header.set(FUNCTION_MKDIRS, timeout);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", name.c_str());
}

void MkdirsSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_mkdirs(ev, name);
  BaseSync::acknowledge();
}



}}}}}

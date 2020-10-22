/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/ReaddirSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


ReaddirSync::ReaddirSync(uint32_t timeout, const std::string& name, 
                         FS::DirentList& listing) 
                        : Base(Buffers::make(Params::ReaddirReq(name))),
                          listing(listing), name(name) {
  cbp->header.set(FUNCTION_READDIR, timeout);
  SWC_LOGF(LOG_DEBUG, "readdir path='%s'", name.c_str());
}

void ReaddirSync::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_readdir(ev, name, listing);
  BaseSync::acknowledge();
}



}}}}}

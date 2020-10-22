/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Exists.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Exists::Exists(uint32_t timeout, const std::string& name, 
               const FS::Callback::ExistsCb_t& cb) 
              : Base(Buffers::make(Params::ExistsReq(name))),
                name(name), cb(cb) {
  cbp->header.set(FUNCTION_EXISTS, timeout);
  SWC_LOGF(LOG_DEBUG, "exists path='%s'", name.c_str());
}


void Exists::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  bool state = false;
  Base::handle_exists(ev, name, state);
  cb(error, state);
}



}}}}}

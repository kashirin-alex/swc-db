/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Rmdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Rmdir::Rmdir(uint32_t timeout, const std::string& name, 
             const FS::Callback::RmdirCb_t& cb) 
            : Base(Buffers::make(Params::RmdirReq(name))),
              name(name), cb(cb) {
  cbp->header.set(FUNCTION_RMDIR, timeout);
  SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());
}

void Rmdir::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_rmdir(ev, name);
  cb(error);
}



}}}}}

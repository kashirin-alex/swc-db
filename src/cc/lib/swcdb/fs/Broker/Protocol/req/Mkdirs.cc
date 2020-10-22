/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Mkdirs.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Mkdirs::Mkdirs(uint32_t timeout, const std::string& name, 
               const FS::Callback::MkdirsCb_t& cb) 
              : Base(Buffers::make(Params::MkdirsReq(name))),
                name(name), cb(cb) {
  cbp->header.set(FUNCTION_MKDIRS, timeout);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", name.c_str());
}

void Mkdirs::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_mkdirs(ev, name);
  cb(error);
}



}}}}}

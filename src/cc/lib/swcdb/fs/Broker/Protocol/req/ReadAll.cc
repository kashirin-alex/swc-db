/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/ReadAll.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {

  
ReadAll::ReadAll(uint32_t timeout, const std::string& name,
                 const FS::Callback::ReadAllCb_t& cb)
                : Base(Buffers::make(Params::ReadAllReq(name))),
                  name(name), cb(cb) {
  cbp->header.set(FUNCTION_READ_ALL, timeout);

  SWC_LOGF(LOG_DEBUG, "read-all timeout=%d %s", timeout, name.c_str());
}


void ReadAll::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  Base::handle_read_all(ev, name);
  StaticBuffer::Ptr buf(error ? nullptr : new StaticBuffer(ev->data_ext));
  cb(error, name, buf);
}



}}}}}

/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/ReadAll.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {

  
ReadAll::ReadAll(uint32_t timeout, const std::string& name, StaticBuffer* dst,
                 const FS::Callback::ReadAllCb_t& cb)
                : buffer(dst), name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "read-all timeout=%d %s", timeout, name.c_str());

  cbp = Buffers::make(Params::ReadAllReq(name));
  cbp->header.set(FUNCTION_READ_ALL, timeout);
}

std::promise<void> ReadAll::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise]
       (int, const std::string&, const StaticBuffer::Ptr&) {
         await->set_value();
        };
  return r_promise;
}

void ReadAll::handle(ConnHandlerPtr, const Event::Ptr& ev) {

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_READ_ALL, &ptr, &remain))
    return;
    
  StaticBuffer::Ptr buf = nullptr;
  if(!error) {
    if(buffer)
      buffer->set(ev->data_ext);
    else
      buf.reset(new StaticBuffer(ev->data_ext));
  }

  SWC_LOGF(LOG_DEBUG, "read-all %s amount='%lu' error='%d'", 
                        name.c_str(), 
                        error ? 0 : (buffer ? buffer->size : buf->size),
                        error);
  cb(error, name, buf);
}



}}}}}

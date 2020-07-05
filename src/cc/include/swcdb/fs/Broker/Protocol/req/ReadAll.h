/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_ReadAll_h
#define swc_fs_Broker_Protocol_req_ReadAll_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/ReadAll.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class ReadAll : public Base {

  public:
  StaticBuffer* buffer;
  
  ReadAll(uint32_t timeout, const std::string& name, StaticBuffer* dst,
          const Callback::ReadAllCb_t& cb=0)
          : buffer(dst), name(name), cb(cb) {
    SWC_LOGF(LOG_DEBUG, "read-all timeout=%d %s", timeout, name.c_str());

    cbp = CommBuf::make(Params::ReadAllReq(name));
    cbp->header.set(Cmd::FUNCTION_READ_ALL, timeout);
  }

  std::promise<void> promise() {
    std::promise<void>  r_promise;
    cb = [await=&r_promise]
         (int err, const std::string& name, StaticBuffer::Ptr buf) {
           await->set_value();
          };
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_READ_ALL, &ptr, &remain))
      return;
      
    StaticBuffer::Ptr buf = nullptr;
    if(!error) {
      if(buffer)
        buffer->set(ev->data_ext);
      else
        buf.reset(new StaticBuffer(ev->data_ext));
    }

    SWC_LOGF(LOG_DEBUG, "read-all %s amount='%u' error='%d'", 
                          name.c_str(), 
                          error ? 0 : (buffer ? buffer->size : buf->size),
                          error);
    cb(error, name, buf);
  }

  private:
  const std::string      name;
  Callback::ReadAllCb_t  cb;
};



}}}}

#endif  // swc_fs_Broker_Protocol_req_ReadAll_h
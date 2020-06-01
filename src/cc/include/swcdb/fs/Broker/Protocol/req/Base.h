/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_req_Base_h
#define swc_fs_Broker_Protocol_req_Base_h

#include "swcdb/core/Serialization.h"
#include "swcdb/core/comm/DispatchHandler.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Base : public DispatchHandler {

  public:

  using Ptr = BasePtr;

  CommBuf::Ptr  cbp;
  int           error;
  bool          was_called;

  Base() : error(Error::OK), was_called(false) {}

  virtual ~Base() {}

  bool is_rsp(ConnHandlerPtr conn, Event::Ptr& ev, int cmd, 
              const uint8_t **ptr, size_t *remain) { 
    // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

    if(was_called)
      return false;

    switch(ev->type){
      case Event::Type::DISCONNECT:
        error = Error::COMM_NOT_CONNECTED;
        break;
      case Event::Type::ERROR:
        error = ev->error;
        break;
      case Event::Type::ESTABLISHED:
        return false;
      default:
        break;
    }

    if(!error && ev->header.command != cmd){
      error = Error::NOT_IMPLEMENTED;
      SWC_LOGF(LOG_ERROR, "error=%d(%s) cmd=%d", 
                error, Error::get_text(error), ev->header.command);

    } else if((!error && !(error = ev->response_code())) || 
              error == Error::FS_EOF) {
      *ptr = ev->data.base + 4;
      *remain = ev->data.size - 4;
    } 
    
    if(error && 
       (cmd != Cmd::FUNCTION_READ_ALL || error != Error::FS_PATH_NOT_FOUND))
      SWC_LOGF(LOG_ERROR, "error=%d(%s)", error, Error::get_text(error));
    
    was_called = true;
    return true;
  }

};



}}}}

#endif  // swc_fs_Broker_Protocol_req_Base_h
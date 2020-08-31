/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 

#include "swcdb/fs/Broker/Protocol/req/Base.h"


namespace SWC { namespace FS { namespace Protocol { namespace Req {


Base::Base() : error(Error::OK) {}

Base::~Base() {}

bool Base::is_rsp(const Event::Ptr& ev, int cmd, 
                  const uint8_t **ptr, size_t *remain) { 
  // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

  switch(ev->type) {
    case Event::Type::ESTABLISHED:
      return false;
    case Event::Type::DISCONNECT:
    case Event::Type::ERROR:
      error = ev->error;
      break;
    default:
      break;
  }

  if(!error) {
    if(ev->header.command != cmd) {
      error = Error::NOT_IMPLEMENTED;
      SWC_LOGF(LOG_ERROR, "error=%d(%s) cmd=%d", 
                error, Error::get_text(error), ev->header.command);
    } else if((!(error = ev->response_code())) || error == Error::FS_EOF) {
      *ptr = ev->data.base + 4;
      *remain = ev->data.size - 4;
    }
  }
  
  if(error && 
     (cmd != Cmd::FUNCTION_READ_ALL || error != Error::FS_PATH_NOT_FOUND))
    SWC_LOGF(LOG_ERROR, "error=%d(%s)", error, Error::get_text(error));
  
  return true;
}



}}}}

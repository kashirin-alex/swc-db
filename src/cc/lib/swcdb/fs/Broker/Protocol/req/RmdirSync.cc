/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/RmdirSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


RmdirSync::RmdirSync(uint32_t timeout, const std::string& name)
                    : Base(Buffers::make(Params::RmdirReq(name))),
                      name(name) {
  cbp->header.set(FUNCTION_RMDIR, timeout);
  SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());
}


void RmdirSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_RMDIR, &ptr, &remain))
    return;

  SWC_LOGF(LOG_DEBUG, "rmdir path='%s' error='%d'", name.c_str(), error);
  
  BaseSync::acknowledge();
}



}}}}}
